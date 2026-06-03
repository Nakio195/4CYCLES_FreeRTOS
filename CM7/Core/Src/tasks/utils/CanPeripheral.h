/*
 * CanPeripheral.h
 *
 *  Created on: Feb 3, 2025
 *      Author: To
 */

/*
 * CanPeripheral.h
 *
 *   [Any state]  ──── Heartbeat ──────────────────────────────► Initialized
 *   Initialized  ──── Packet received ───────────────────────► Ready
 *   Initialized  ──── Timeout, retry < MAX ─────────────────► Initialized  (resend settings)
 *   Initialized  ──── Timeout, retry >= MAX ────────────────► Absent ──► (reset) Uninitialized
 *   Ready        ──── Timeout ─────────────────────────────► Recovery
 *   Recovery     ──── Packet received ───────────────────────► Ready
 *   Recovery     ──── retry >= MAX ─────────────────────────► Lost ───► (reset) Uninitialized
 *
 * Thread safety:
 *   push()      — may be called from a separate CAN receive task concurrently with tick().
 *   heartbeat() — called from the peripheral's own task (inside run()).
 *   tick()      — called from the peripheral's own task (inside run()).
 *
 *
 *
 * API changes vs original (update subclasses accordingly):
 *   init()       → onInit()       — no longer calls CanPeripheral::init() to advance state
 *   discovered() → onDiscovered()
 *   recovery()   → onRecovery()   — replaces reInit() call; just resend settings here
 *   recovered()  → onRecovered()
 *   lost()       → onLost()
 *   absent()     → onAbsent()
 *   reInit()        removed       — fold logic into onRecovery()
 */

#ifndef SRC_TASKS_UTILS_CANPERIPHERAL_H_
#define SRC_TASKS_UTILS_CANPERIPHERAL_H_

#include <inttypes.h>
#include <cmsis_os2.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

#include "LockGuard.hpp"
#include "CanPacket.h"
#include "Message.h"
#include "Event.h"

class CanPeripheral
{
	public:
		enum FilterMode     : uint8_t { Range, Mask };
		enum State          : uint8_t { Uninitialized, Initialized, Ready, Recovery, Lost, Absent, Disabled };
		enum PeripheralType : uint8_t { Controller, Logger, Battery, Accessory };
		enum PeripheralId   : uint8_t { Unknown, HandlebarController, RemoteController, BatteryTYVA, MainLogger, Count };

	public:
		CanPeripheral()
		{
			mState = Uninitialized;
			mRetryCount = 0;
			mMaxRetries = 3;
			mRecoveryTicks = 100;
			mResetDelayTicks = 1000;
			mPreviousTick = 0;
			mLastCommunication = 0;
			mCommunicationTimeout = 500;
			mLastHeartbeat = 0;
			mHeartbeat = 0;
			mDtHeartbeat = 0;
			mPeripheralId = Unknown;
			mPeripheralType = Controller;
			mFilterMode = Range;
			mFilterLow = 0;
			mFilterHigh = 0;
			mFilterId = 0;
			mFilterMask = 0;
			mHeartbeatRequired = true;

			mMutex        = xSemaphoreCreateMutex();
			mPacketsQueue = xQueueCreate(16, sizeof(CanPacket*));
			xSemaphoreGive(mMutex);
		}

		// ── Public API ────────────────────────────────────────────────────────

		/**
		 * Called by the CAN receive task when an incoming frame matches this peripheral.
		 * Thread-safe: may run concurrently with tick().
		 *
		 * Atomically resets the watchdog and, if we were waiting for a response
		 * (Initialized or Recovery), promotes the state to Ready inside the same
		 * critical section — no TOCTOU window.
		 */
		bool push(CanPacket* packet)
		{
			if(!accept(packet->Identifier))
				return false;

			State prev;
			bool  promoted  = false;
			bool  doInit    = false;
			bool  queued    = false;

			{
				LockGuard lock(mMutex);

				if(mState == Disabled)
				{
					// While disabled: only reset the watchdog so heartbeat
					// monitoring stays alive. Business data is discarded.
					mLastCommunication = 0;
					return false;
				}

				mLastCommunication = 0;

				if(mState == Initialized || mState == Recovery)
				{
					prev        = mState;
					mState      = Ready;
					mRetryCount = 0;
					promoted    = true;
				}
				else if(mState == Uninitialized && !mHeartbeatRequired)
				{
					// Peripheral has no heartbeat: first received frame acts as
					// presence signal. transition(Initialized) is called outside
					// the lock (it re-acquires internally).
					doInit = true;
				}

				// Queue under the same lock to prevent double-access
				queued = (xQueueSend(mPacketsQueue, &packet, 0) == pdTRUE);
			}

			// Callbacks fire OUTSIDE the lock — they may themselves emit events or log
			if(promoted)
			{
				if(prev == Recovery) onRecovered();
				else                 onDiscovered();
			}

			else if(doInit)
			{
				transition(Initialized); // validates mState == Uninitialized, calls onInit()
			}

			return queued;
		}

		State status()
		{
			LockGuard lock(mMutex);
			return mState;
		}

		void setRecoveryMode(uint8_t maxRetries, uint16_t retryTicks)
		{
			mMaxRetries    = maxRetries;
			mRecoveryTicks = retryTicks;
		}

		/**
		 * Call this in the subclass constructor for peripherals that do not emit
		 * a dedicated heartbeat frame. Any matching packet will then act as a
		 * presence signal and trigger Uninitialized → Initialized automatically.
		 * Timeout monitoring (Lost/Absent) remains active as normal.
		 */
		void setHeartbeatRequired(bool required)
		{
			mHeartbeatRequired = required;
		}

		/**
		 * Disable the peripheral. Transitions to Disabled from any state.
		 * Business data packets will be discarded; heartbeat monitoring continues.
		 * Thread-safe.
		 */
		void disable()
		{
			transition(Disabled);
		}

		/**
		 * Re-enable the peripheral.
		 * If a heartbeat was received recently (device is present), triggers
		 * initialization immediately (Initialized → onInit()).
		 * Otherwise resets to Uninitialized and waits for the next heartbeat.
		 * Thread-safe.
		 */
		void enable()
		{
			bool present;
			{
				LockGuard lock(mMutex);
				if(mState != Disabled) return;
				present = (mLastCommunication < mCommunicationTimeout);
			}

			if(present) transition(Initialized);
			else        transition(Uninitialized);
		}

	protected:
		// ── Subclass callbacks ────────────────────────────────────────────────

		/**
		 * Entering Initialized: send initial configuration settings to the device.
		 * Called when the device (re)appears after Uninitialized, Lost, or Absent.
		 * Also called on timeout retries while still in Initialized state.
		 * A heartbeat received in Ready or Recovery does NOT trigger this.
		 */
		virtual void onInit()       = 0;

		/**
		 * Entering Ready from Initialized: first successful response received.
		 */
		virtual void onDiscovered() = 0;

		/**
		 * One recovery attempt: resend settings. Called once per retry interval.
		 * Return quickly; blocking is handled by the recovery loop in tick().
		 */
		virtual void onRecovery()   = 0;

		/**
		 * Entering Ready from Recovery: communication re-established.
		 */
		virtual void onRecovered()  = 0;

		/**
		 * Entering Lost: max retries exhausted; device was previously operational.
		 * Emit a disconnect event here if needed.
		 */
		virtual void onLost()       = 0;

		/**
		 * Entering Absent: max retries exhausted; device never reached Ready.
		 * Device is considered optional / not present.
		 */
		virtual void onAbsent()     = 0;

		/**
		 * Entering Disabled: clear any active commands and emit a disconnect event
		 * if the device was previously operational. Heartbeat monitoring continues
		 * in the background; business data packets will be discarded.
		 */
		virtual void onDisabled()   = 0;

		// ─────────────────────────────────────────────────────────────────────

		/**
		 * Call this when a heartbeat frame is decoded in run().
		 *
		 * The heartbeat is emitted permanently by the device (every ~1 s), so its
		 * effect on the state machine depends on context:
		 *
		 *   Lost / Absent / Uninitialized → transition(Initialized)
		 *     The device (re)appeared; send settings before accepting data.
		 *
		 *   Initialized → reset watchdog + retry counter only.
		 *     Already initializing; don't re-send settings on every heartbeat.
		 *
		 *   Ready / Recovery → update heartbeat tracking only.
		 *     push() already reset mLastCommunication for the heartbeat packet and
		 *     handles the Initialized/Recovery → Ready promotion atomically.
		 *     No state change needed here.
		 */
		void heartbeat(uint32_t hb)
		{
			bool doInit = false;
			{
				LockGuard lock(mMutex);
				mLastHeartbeat = mHeartbeat;
				mHeartbeat     = hb;
				mDtHeartbeat   = mHeartbeat - mLastHeartbeat;

				switch(mState)
				{
					case Uninitialized:
					case Lost:
					case Absent:
						doInit = true;  // device (re)appeared → full init
						break;

					case Initialized:
						// Already initializing; just refresh the watchdog
						mLastCommunication = 0;
						mRetryCount        = 0;
						break;

					case Ready:
					case Recovery:
						// Normal operation or active recovery; push() handles watchdog
						// and state promotion — nothing to do here.
						break;

					case Disabled:
						// Heartbeat confirms the device is still physically present.
						// push() already reset mLastCommunication; no state change.
						break;
				}
			}

			if(doInit)
				transition(Initialized); // calls onInit() outside lock
		}

		/**
		 * Periodic state machine tick. Call once per task iteration, passing
		 * xTaskGetTickCount(). Must NOT be called with mMutex held.
		 *
		 * Responsibilities:
		 *   1. Advance the communication watchdog.
		 *   2. Handle timeout → Recovery or Initialized retry/Absent.
		 *   3. Handle Lost/Absent → Uninitialized reset (with delay, outside lock).
		 *   4. Run the blocking recovery retry loop (entirely lock-free so push() fires).
		 */
		void tick(uint32_t t)
		{
			// ── 1. Advance watchdog ───────────────────────────────────────────
			bool  timedOut       = false;
			State stateAtTimeout = Uninitialized;
			{
				LockGuard lock(mMutex);
				mLastCommunication += t - mPreviousTick;
				mPreviousTick       = t;

				// Timeout is meaningless in Recovery (has its own retry cadence),
				// in Uninitialized (we haven't started yet), and in Disabled
				// (no transitions are triggered while the peripheral is off).
				if(mLastCommunication >= mCommunicationTimeout
				   && mState != Recovery
				   && mState != Uninitialized
				   && mState != Disabled)
				{
					timedOut       = true;
					stateAtTimeout = mState; // capture state at the moment of timeout
				}
			}

			// ── 2. Handle timeout (outside lock) ─────────────────────────────
			if(timedOut)
			{
				if(stateAtTimeout == Ready)
				{
					// transition() re-validates mState == Ready inside its own lock,
					// so a concurrent heartbeat that moved us to Initialized is safe.
					transition(Recovery);
				}
				else if(stateAtTimeout == Initialized)
				{
					bool doAbsent = false;
					bool doResend = false;
					{
						LockGuard lock(mMutex);
						// Re-check: a heartbeat may have already reset mState/mRetryCount
						if(mState == Initialized)
						{
							if(++mRetryCount >= mMaxRetries) doAbsent = true;
							else                             doResend = true;
						}
					}
					if(doAbsent)      transition(Absent);
					else if(doResend) onInit(); // resend settings, stay in Initialized
				}
			}

			// ── 3. Lost / Absent → Uninitialized reset ───────────────────────
			// osDelay is OUTSIDE the lock — push() is never starved during this pause.
			State current = status();
			if(current == Lost || current == Absent)
			{
				osDelay(mResetDelayTicks);
				transition(Uninitialized); // validated: only from Lost or Absent
				return;
			}

			// ── 4. Recovery retry loop ────────────────────────────────────────
			// The loop holds NO lock so push() can fire at any point and
			// atomically move mState to Ready, breaking the loop naturally.
			if(status() == Recovery)
			{
				bool exhausted = false;

				while(status() == Recovery)
				{
					onRecovery();          // subclass resends settings
					osDelay(mRecoveryTicks);

					{
						LockGuard lock(mMutex);
						// Only increment if push() hasn't already rescued us
						if(mState == Recovery && ++mRetryCount >= mMaxRetries)
						{
							exhausted = true;
							break;
						}
					}
				}

				// If push() moved us to Ready, transition(Lost) below is a no-op
				// (predecessor check inside transition() will reject Recovery→Lost
				//  when mState is already Ready).
				if(exhausted)
					transition(Lost);
			}
		}

		void setCommunicationTimeout(uint32_t timeout)
		{
			LockGuard lock(mMutex);
			mCommunicationTimeout = timeout;
		}

		void setRangeFilter(uint32_t low, uint32_t high)
		{
			mFilterMode = Range;
			mFilterLow  = low;
			mFilterHigh = high;
		}

		void setMaskFilter(uint32_t id, uint32_t mask)
		{
			mFilterMode = Mask;
			mFilterId   = id;
			mFilterMask = mask;
		}

	private:
		// ── Core state machine ────────────────────────────────────────────────

		/**
		 * Central transition function.
		 *
		 * Atomically:
		 *   - Validates that the current state is a legal predecessor for `next`
		 *     (stale timeouts or recovery events after a concurrent heartbeat are
		 *     silently discarded).
		 *   - Writes the new state, resets mRetryCount and mLastCommunication.
		 *
		 * Then fires the appropriate entry-action callback OUTSIDE the lock.
		 *
		 * Valid transitions:
		 *   Any          → Initialized   (heartbeat — universal reset)
		 *   Initialized  → Ready
		 *   Initialized  → Absent
		 *   Ready        → Recovery
		 *   Recovery     → Ready         (only via push(), not here)
		 *   Recovery     → Lost
		 *   Lost         → Uninitialized
		 *   Absent       → Uninitialized
		 */
		void transition(State next)
		{
			State prev  = Uninitialized;
			bool  valid = false;

			{
				LockGuard lock(mMutex);

				switch(next)
				{
					case Initialized:
						// Heartbeat resets from Uninitialized, Lost, Absent.
						// enable() may also reach Initialized directly from Disabled
						// when the device is already present (mLastCommunication fresh).
						valid = (mState != Ready && mState != Recovery);
						break;
					case Ready:
						valid = (mState == Initialized || mState == Recovery);
						break;
					case Recovery:
						valid = (mState == Ready);
						break;
					case Lost:
						valid = (mState == Recovery);
						break;
					case Absent:
						valid = (mState == Initialized);
						break;
					case Uninitialized:
						// Auto-reset after Lost/Absent, or explicit enable()
						valid = (mState == Lost || mState == Absent || mState == Disabled);
						break;
					case Disabled:
						valid = true; // disable() accepted from any state
						break;
				}

				if(valid)
				{
					prev               = mState;
					mState             = next;
					mRetryCount        = 0;
					mLastCommunication = 0;
				}
			}

			if(!valid) return;

			// Entry actions — called outside lock
			switch(next)
			{
				case Initialized:   onInit();       break;
				case Ready:
					if(prev == Recovery) onRecovered();
					else                 onDiscovered();
					break;
				case Lost:          onLost();       break;
				case Absent:        onAbsent();     break;
				case Disabled:      onDisabled();   break;
				default:            break; // Uninitialized and Recovery have no entry callback
			}
		}

		bool accept(uint32_t id) const
		{
			if(mFilterMode == Range) return id >= mFilterLow && id <= mFilterHigh;
			if(mFilterMode == Mask)  return (id & mFilterMask) == (mFilterId & mFilterMask);
			return false;
		}

	protected:
		// Accessible to subclasses
		SemaphoreHandle_t mMutex;
		QueueHandle_t     mPacketsQueue;

		uint8_t  mPeripheralId;
		uint8_t  mPeripheralType;

		// Heartbeat tracking (read-only from subclasses)
		uint32_t mLastHeartbeat;
		uint32_t mHeartbeat;
		uint32_t mDtHeartbeat;

	private:
		// State machine
		State    mState;
		bool     mHeartbeatRequired; // false → any packet can trigger Uninitialized→Initialized

		uint8_t  mRetryCount;        // shared between Initialized retries and Recovery
		uint8_t  mMaxRetries;
		uint16_t mRecoveryTicks;     // ms between recovery attempts
		uint16_t mResetDelayTicks;   // ms to wait before Uninitialized reset after Lost/Absent

		// Watchdog timing
		uint32_t mPreviousTick;
		uint32_t mLastCommunication; // ms since last received packet; reset on any packet
		uint32_t mCommunicationTimeout;

		// CAN ID filtering
		uint8_t  mFilterMode;
		uint32_t mFilterLow;
		uint32_t mFilterHigh;
		uint32_t mFilterId;
		uint32_t mFilterMask;
};


#endif
