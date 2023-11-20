#include "pch.h"
#include "LifetimeManager.h"
#include "main.h"

namespace winrt::HotCorner::Server::implementation {
	LifetimeManager::LifetimeManager() noexcept :
		m_icon(Tracking::TrayCornerTracker::Current()) { }

	void LifetimeManager::OnWaited(PVOID param, BOOLEAN) {
		static_cast<LifetimeManager*>(param)->Unregister();
	}

	winrt::fire_and_forget LifetimeManager::Unregister() noexcept {
		co_await std::chrono::seconds(0);
		ReleaseServer();

		Release();
	}

	void LifetimeManager::LockServer(uint32_t pid) {
		const auto proc = OpenProcess(SYNCHRONIZE, false, pid);

		if (!proc) {
			throw winrt::hresult_invalid_argument(L"The provided process ID is invalid.");
		}

		const bool registered = RegisterWaitForSingleObject(
			&m_waitHandle,
			proc,
			&LifetimeManager::OnWaited,
			this,
			INFINITE,
			WT_EXECUTEONLYONCE
		);

		winrt::check_bool(registered);
		BumpServer();
	}

	void LifetimeManager::ReloadSettings() {
		const auto result = m_icon.StopTracking();
		Current::Settings().Load();

		if (result == CornerTracker::StopResult::Stopped) {
			m_icon.BeginTracking();
		}
		else if (result == CornerTracker::StopResult::Failed) {
			//TODO: Handle failure
			OutputDebugString(L"Failed to stop corner tracking\n");
		}
	}

	void LifetimeManager::TrackHotCorners() const noexcept {
		const auto result = m_icon.BeginTracking();

		if (result == CornerTracker::StartupResult::Started) {
			BumpServer();
		}
		else if (result == CornerTracker::StartupResult::Failed) {
			//TODO: Handle failure
			OutputDebugString(L"Failed to start corner tracking\n");
		}
	}

	void LifetimeManager::StopTracking() const noexcept {
		const auto result = m_icon.StopTracking();

		if (result == CornerTracker::StopResult::Stopped) {
			ReleaseServer();
		}
		else if (result == CornerTracker::StopResult::Failed) {
			//TODO: Handle failure
			OutputDebugString(L"Failed to stop corner tracking\n");
		}
	}

	void LifetimeManager::ShowTrayIcon() const noexcept {
		if (!m_icon.Visible()) {
			m_icon.Show();
			BumpServer();
		}
	}

	void LifetimeManager::HideTrayIcon() const noexcept {
		if (m_icon.Visible()) {
			m_icon.Hide();
			ReleaseServer();
		}
	}

	LifetimeManager::~LifetimeManager() noexcept {
		if (m_waitHandle) {
			const auto unregistered = UnregisterWait(m_waitHandle);
			if (!unregistered) {
				//TODO: Handle failure
				OutputDebugString(L"Failed to unregister wait handle\n");
			}
		}
	}
}
