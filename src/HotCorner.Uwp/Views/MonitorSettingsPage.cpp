﻿#include "pch.h"
#include "MonitorSettingsPage.h"
#include "Views/MonitorSettingsPage.g.cpp"

namespace wfc = winrt::Windows::Foundation::Collections;
namespace wux = winrt::Windows::UI::Xaml;
namespace wuxc = winrt::Windows::UI::Xaml::Controls;

using App = winrt::HotCorner::Uwp::implementation::App;

namespace winrt::HotCorner::Uwp::Views::implementation {
	//TODO: A better way of doing this? Or at least move it to its own header?
	IInspectable operator ""_box(const wchar_t* str, std::size_t size) {
		return box_value(hstring{ { str, size } });
	}

	wfc::IVectorView<IInspectable> MonitorSettingsPage::CornerActions() {
		static const auto m_cornerActions = winrt::single_threaded_vector<IInspectable>(
			{ L"None"_box, L"Open Task View"_box, L"Open Start"_box, L"Open Search"_box, L"Go to Desktop"_box }
		).GetView();
		return m_cornerActions;
	}

	void MonitorSettingsPage::OnMonitorPropertyChanged(const wux::DependencyObject& sender, const wux::DependencyPropertyChangedEventArgs& args) {
		const auto page = sender.as<MonitorSettingsPage>();
		const auto val = args.NewValue().as<Devices::MonitorInfo>();
		const auto id = val.Id().c_str();

		auto& saved = App::Settings().Monitors;
		const auto stored = std::find_if(saved.begin(), saved.end(), [&id](const Settings::MonitorSettings& monitor)
			{
				return monitor.Id == id;
			}
		);

		// Is a monitor with this ID already in settings? If yes, load that
		if (stored != saved.end()) {
			page->Refresh(std::distance(saved.begin(), stored));
		}
		else {
			Settings::MonitorSettings newSettings(id, val.DisplayName().c_str());
			saved.push_back(newSettings);
			page->Refresh(saved.size() - 1);
		}
	}

	Settings::MonitorSettings& MonitorSettingsPage::CurrentSettings() const {
		return App::Settings().Monitors[m_currentSettings];
	}

	void UpdateSelection(const wuxc::ComboBox& box, Settings::CornerAction act) {
		if (act != Settings::CornerAction::None) {
			box.SelectedIndex(static_cast<int32_t>(act));
		}
		else {
			box.SelectedIndex(-1);
		}
	}

	void MonitorSettingsPage::Refresh(size_t index) {
		m_currentSettings = index;
		const auto& curr = CurrentSettings();

		UpdateSelection(TopLeftCorner(), curr.TopLeftAction);
		UpdateSelection(TopRightCorner(), curr.TopRightAction);
		UpdateSelection(BottomLeftCorner(), curr.BottomLeftAction);
		UpdateSelection(BottomRightCorner(), curr.BottomRightAction);

		GlobalCheck().IsChecked(curr.Enabled);
	}

	void MonitorSettingsPage::InitializeComponent() {
		MonitorSettingsPageT::InitializeComponent();
		const auto items = CornerActions();

		TopLeftCorner().ItemsSource(items);
		TopRightCorner().ItemsSource(items);
		BottomLeftCorner().ItemsSource(items);
		BottomRightCorner().ItemsSource(items);
	}

	void MonitorSettingsPage::OnGlobalCheckClick(const IInspectable&, const wux::RoutedEventArgs&) {
		CurrentSettings().Enabled = GlobalCheck().IsChecked().GetBoolean();
	}

	void MonitorSettingsPage::OnActionSelected(const wuxc::ComboBox& box, Settings::CornerAction& action) const {
		const auto index = box.SelectedIndex();

		const auto newAction = index < 1 ?
			Settings::CornerAction::None : static_cast<Settings::CornerAction>(index);
		action = newAction;
	}

	void MonitorSettingsPage::OnTopLeftActionSelected(const IInspectable&, const wuxc::SelectionChangedEventArgs&) {
		OnActionSelected(TopLeftCorner(), CurrentSettings().TopLeftAction);
	}
	void MonitorSettingsPage::OnTopRightActionSelected(const IInspectable&, const wuxc::SelectionChangedEventArgs&) {
		OnActionSelected(TopRightCorner(), CurrentSettings().TopRightAction);
	}
	void MonitorSettingsPage::OnBottomLeftActionSelected(const IInspectable&, const wuxc::SelectionChangedEventArgs&) {
		OnActionSelected(BottomLeftCorner(), CurrentSettings().BottomLeftAction);
	}
	void MonitorSettingsPage::OnBottomRightActionSelected(const IInspectable&, const wuxc::SelectionChangedEventArgs&) {
		OnActionSelected(BottomRightCorner(), CurrentSettings().BottomRightAction);
	}
}