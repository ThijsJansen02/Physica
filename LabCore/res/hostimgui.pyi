from __future__ import annotations
from typing import Tuple, List, overload

# -------------------------
# Math types
# -------------------------

class Vec2:
    def __init__(self, x: float = 0.0, y: float = 0.0) -> None: ...
    x: float
    y: float

class Vec4:
    def __init__(self, x: float = 0.0, y: float = 0.0, z: float = 0.0, w: float = 0.0) -> None: ...
    x: float
    y: float
    z: float
    w: float

class Col(IntEnum):
    Text: int
    TextDisabled: int
    WindowBg: int
    ChildBg: int
    PopupBg: int
    Border: int
    FrameBg: int
    FrameBgHovered: int
    FrameBgActive: int
    TitleBg: int
    TitleBgActive: int
    Button: int
    ButtonHovered: int
    ButtonActive: int
    Header: int
    HeaderHovered: int
    HeaderActive: int
    Tab: int
    TabHovered: int
    TabActive: int
    TabUnfocussed : int
	TabSelected : int
    TabSelectedOverline : int
    TabDimmedSelectedOverline : int


# -------------------------
# ImGui Style
# -------------------------

class Style:
    alpha: float
    disabled_alpha: float

    window_rounding: float
    window_border_size: float
    child_border_size: float
    popup_border_size: float

    frame_rounding: float
    frame_border_size: float

    indent_spacing: float
    columns_min_spacing: float

    scrollbar_size: float
    scrollbar_rounding: float

    grab_min_size: float
    grab_rounding: float

    log_slider_deadzone: float

    tab_rounding: float
    tab_border_size: float
    tab_min_width_for_unselected_close_button: float

    mouse_cursor_scale: float

    curve_tessellation_tol: float
    circle_tessellation_max_error: float

    # enum-like (int backed)
    window_menu_button_position: int
    color_button_position: int

    # bools
    anti_aliased_lines: bool
    anti_aliased_lines_use_tex: bool
    anti_aliased_fill: bool

    # Vec2 fields
    window_padding: Vec2
    window_min_size: Vec2
    window_title_align: Vec2
    frame_padding: Vec2
    item_spacing: Vec2
    item_inner_spacing: Vec2
    cell_padding: Vec2
    touch_extra_padding: Vec2
    display_window_padding: Vec2
    display_safe_area_padding: Vec2
    selectable_text_align: Vec2


# -------------------------
# Style API
# -------------------------

def getStyle() -> Style:
    """Returns live ImGui style reference."""
    ...

# -------------------------
# Colors API
# -------------------------

def getColor(index: int) -> Vec4:
    """Get style color by ImGuiCol_* index."""
    ...

def setColor(index: int, color: Vec4) -> None:
    """Set style color by ImGuiCol_* index."""
    ...


# -------------------------
# Optional UI API (if you exposed it)
# -------------------------

def begin(name: str) -> bool: ...
def end() -> None: ...

def text(content: str) -> None: ...
def button(label: str) -> bool: ...