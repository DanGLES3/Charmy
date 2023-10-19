#include "pch.h"
#include "Lifetime.h"

namespace winrt::HotCorner::Uwp::Lifetime {
	namespace {
		Server::LifetimeManager m_lifetime{ nullptr };
	}

	const Server::LifetimeManager& Current() noexcept {
		if (!m_lifetime) {
			m_lifetime = winrt::create_instance<Server::LifetimeManager>(
				guid{ L"898F12B7-4BB0-4279-B3B1-126440D7CB7A" }, CLSCTX_LOCAL_SERVER
			);
			m_lifetime.LockServer(GetCurrentProcessId());
		}
		return m_lifetime;
	}
}
