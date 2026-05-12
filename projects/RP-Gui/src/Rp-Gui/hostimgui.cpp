#ifdef _DEBUG
	#undef _DEBUG
	#include <Python.h>
	#define _DEBUG
	#else
	#include <Python.h>
#endif


#include <imgui.h>
#include <pybind11/embed.h>
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>


namespace py = pybind11;



PYBIND11_EMBEDDED_MODULE(hostimgui, m) {

	m.doc() = "exposed imgui function for use in python";

	py::class_<ImVec2>(m, "Vec2")
		.def(py::init<float, float>())
		.def_readwrite("x", &ImVec2::x)
		.def_readwrite("y", &ImVec2::y);

	py::class_<ImVec4>(m, "Vec4")
		.def(py::init<float, float, float, float>())
		.def_readwrite("x", &ImVec4::x)
		.def_readwrite("y", &ImVec4::y)
		.def_readwrite("z", &ImVec4::z)
		.def_readwrite("w", &ImVec4::w);

	py::enum_<ImGuiCol_>(m, "Col")
		.value("Text", ImGuiCol_Text)
		.value("TextDisabled", ImGuiCol_TextDisabled)
		.value("WindowBg", ImGuiCol_WindowBg)
		.value("ChildBg", ImGuiCol_ChildBg)
		.value("PopupBg", ImGuiCol_PopupBg)
		.value("Border", ImGuiCol_Border)
		.value("FrameBg", ImGuiCol_FrameBg)
		.value("FrameBgHovered", ImGuiCol_FrameBgHovered)
		.value("FrameBgActive", ImGuiCol_FrameBgActive)
		.value("TitleBg", ImGuiCol_TitleBg)
		.value("TitleBgActive", ImGuiCol_TitleBgActive)
		.value("Button", ImGuiCol_Button)
		.value("ButtonHovered", ImGuiCol_ButtonHovered)
		.value("ButtonActive", ImGuiCol_ButtonActive)
		.value("Header", ImGuiCol_Header)
		.value("HeaderHovered", ImGuiCol_HeaderHovered)
		.value("HeaderActive", ImGuiCol_HeaderActive)
		.value("Tab", ImGuiCol_Tab)
		.value("TabActive", ImGuiCol_TabActive)
		.value("TabHovered", ImGuiCol_TabHovered)
		.value("TabSelected", ImGuiCol_TabSelected)
		.value("TabUnfocussed", ImGuiCol_TabUnfocused)
		.value("TabDimmed", ImGuiCol_TabDimmed)
		.value("TabDimmedSelectedOverline", ImGuiCol_TabDimmedSelectedOverline)
		.value("TabSelectedOverline", ImGuiCol_TabSelectedOverline)
		.export_values();

	py::class_<ImGuiStyle>(m, "Style")
		// --- main scalar values ---
		.def_readwrite("alpha", &ImGuiStyle::Alpha)
		.def_readwrite("disabled_alpha", &ImGuiStyle::DisabledAlpha)
		.def_readwrite("window_rounding", &ImGuiStyle::WindowRounding)
		.def_readwrite("window_border_size", &ImGuiStyle::WindowBorderSize)
		.def_readwrite("child_border_size", &ImGuiStyle::ChildBorderSize)
		.def_readwrite("popup_border_size", &ImGuiStyle::PopupBorderSize)
		.def_readwrite("frame_rounding", &ImGuiStyle::FrameRounding)
		.def_readwrite("frame_border_size", &ImGuiStyle::FrameBorderSize)
		.def_readwrite("indent_spacing", &ImGuiStyle::IndentSpacing)
		.def_readwrite("columns_min_spacing", &ImGuiStyle::ColumnsMinSpacing)
		.def_readwrite("scrollbar_size", &ImGuiStyle::ScrollbarSize)
		.def_readwrite("scrollbar_rounding", &ImGuiStyle::ScrollbarRounding)
		.def_readwrite("grab_min_size", &ImGuiStyle::GrabMinSize)
		.def_readwrite("grab_rounding", &ImGuiStyle::GrabRounding)
		.def_readwrite("log_slider_deadzone", &ImGuiStyle::LogSliderDeadzone)
		.def_readwrite("tab_rounding", &ImGuiStyle::TabRounding)
		.def_readwrite("tab_border_size", &ImGuiStyle::TabBorderSize)
		.def_readwrite("mouse_cursor_scale", &ImGuiStyle::MouseCursorScale)

		// --- alignment / enums (stored as ints) ---
		.def_readwrite("window_menu_button_position",
			&ImGuiStyle::WindowMenuButtonPosition)
		.def_readwrite("color_button_position",
			&ImGuiStyle::ColorButtonPosition)

		// --- booleans ---
		.def_readwrite("anti_aliased_lines", &ImGuiStyle::AntiAliasedLines)
		.def_readwrite("anti_aliased_lines_use_tex",
			&ImGuiStyle::AntiAliasedLinesUseTex)
		.def_readwrite("anti_aliased_fill", &ImGuiStyle::AntiAliasedFill)

		// --- vec2 fields ---
		.def_readwrite("window_padding", &ImGuiStyle::WindowPadding)
		.def_readwrite("window_min_size", &ImGuiStyle::WindowMinSize)
		.def_readwrite("window_title_align", &ImGuiStyle::WindowTitleAlign)
		.def_readwrite("frame_padding", &ImGuiStyle::FramePadding)
		.def_readwrite("item_spacing", &ImGuiStyle::ItemSpacing)
		.def_readwrite("item_inner_spacing", &ImGuiStyle::ItemInnerSpacing)
		.def_readwrite("cell_padding", &ImGuiStyle::CellPadding)
		.def_readwrite("touch_extra_padding", &ImGuiStyle::TouchExtraPadding)
		.def_readwrite("display_window_padding", &ImGuiStyle::DisplayWindowPadding)
		.def_readwrite("display_safe_area_padding", &ImGuiStyle::DisplaySafeAreaPadding)
		.def_readwrite("selectable_text_align", &ImGuiStyle::SelectableTextAlign)

		// --- curve tuning ---
		.def_readwrite("curve_tessellation_tol",
			&ImGuiStyle::CurveTessellationTol)
		.def_readwrite("circle_tessellation_max_error",
			&ImGuiStyle::CircleTessellationMaxError);

	m.def("getStyle", []() -> ImGuiStyle& {
		return ImGui::GetStyle();
		}, py::return_value_policy::reference);

	m.def("getColor", [](ImGuiCol_ col) -> ImVec4& {
		return ImGui::GetStyle().Colors[col];
		}, py::return_value_policy::reference);

	m.def("setColor", [](ImGuiCol_ col, ImVec4 c) {
		ImGui::GetStyle().Colors[col] = c;
		});
}