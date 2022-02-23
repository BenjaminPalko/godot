/*************************************************************************/
/*  tile_map_editor.h                                                    */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#ifndef TILE_MAP_EDITOR_H
#define TILE_MAP_EDITOR_H

#include "tile_atlas_view.h"

#include "core/os/thread.h"
#include "core/typedefs.h"
#include "scene/2d/tile_map.h"
#include "scene/gui/box_container.h"
#include "scene/gui/check_box.h"
#include "scene/gui/item_list.h"
#include "scene/gui/menu_button.h"
#include "scene/gui/separator.h"
#include "scene/gui/spin_box.h"
#include "scene/gui/split_container.h"
#include "scene/gui/tab_bar.h"
#include "scene/gui/tree.h"

class UndoRedo;

class TileMapEditorPlugin : public Object {
public:
	struct TabData {
		Control *toolbar = nullptr;
		Control *panel = nullptr;
	};

	virtual Vector<TabData> get_tabs() const {
		return Vector<TabData>();
	};

	virtual bool forward_canvas_gui_input(const Ref<InputEvent> &p_event) { return false; };
	virtual void forward_canvas_draw_over_viewport(Control *p_overlay){};
	virtual void tile_set_changed(){};
	virtual void edit(ObjectID p_tile_map_id, int p_tile_map_layer){};
};

class TileMapEditorTilesPlugin : public TileMapEditorPlugin {
	GDCLASS(TileMapEditorTilesPlugin, TileMapEditorPlugin);

private:
	UndoRedo *undo_redo = nullptr;
	ObjectID tile_map_id;
	int tile_map_layer = -1;
	virtual void edit(ObjectID p_tile_map_id, int p_tile_map_layer) override;

	///// Toolbar /////
	HBoxContainer *toolbar = nullptr;

	Ref<ButtonGroup> tool_buttons_group;
	Button *select_tool_button = nullptr;
	Button *paint_tool_button = nullptr;
	Button *line_tool_button = nullptr;
	Button *rect_tool_button = nullptr;
	Button *bucket_tool_button = nullptr;

	HBoxContainer *tools_settings = nullptr;

	VSeparator *tools_settings_vsep = nullptr;
	Button *picker_button = nullptr;
	Button *erase_button = nullptr;

	VSeparator *tools_settings_vsep_2 = nullptr;
	CheckBox *bucket_contiguous_checkbox = nullptr;
	Button *random_tile_toggle = nullptr;
	float scattering = 0.0;
	Label *scatter_label = nullptr;
	SpinBox *scatter_spinbox = nullptr;
	void _on_random_tile_checkbox_toggled(bool p_pressed);
	void _on_scattering_spinbox_changed(double p_value);

	void _update_toolbar();

	///// Tilemap editing. /////
	bool has_mouse = false;
	void _mouse_exited_viewport();

	enum DragType {
		DRAG_TYPE_NONE = 0,
		DRAG_TYPE_SELECT,
		DRAG_TYPE_MOVE,
		DRAG_TYPE_PAINT,
		DRAG_TYPE_LINE,
		DRAG_TYPE_RECT,
		DRAG_TYPE_BUCKET,
		DRAG_TYPE_PICK,
		DRAG_TYPE_CLIPBOARD_PASTE,
	};
	DragType drag_type = DRAG_TYPE_NONE;
	bool drag_erasing = false;
	Vector2 drag_start_mouse_pos;
	Vector2 drag_last_mouse_pos;
	HashMap<Vector2i, TileMapCell> drag_modified;

	TileMapCell _pick_random_tile(Ref<TileMapPattern> p_pattern);
	HashMap<Vector2i, TileMapCell> _draw_line(Vector2 p_start_drag_mouse_pos, Vector2 p_from_mouse_pos, Vector2 p_to_mouse_pos, bool p_erase);
	HashMap<Vector2i, TileMapCell> _draw_rect(Vector2i p_start_cell, Vector2i p_end_cell, bool p_erase);
	HashMap<Vector2i, TileMapCell> _draw_bucket_fill(Vector2i p_coords, bool p_contiguous, bool p_erase);
	void _stop_dragging();

	///// Selection system. /////
	RBSet<Vector2i> tile_map_selection;
	Ref<TileMapPattern> tile_map_clipboard;
	Ref<TileMapPattern> selection_pattern;
	void _set_tile_map_selection(const TypedArray<Vector2i> &p_selection);
	TypedArray<Vector2i> _get_tile_map_selection() const;

	RBSet<TileMapCell> tile_set_selection;

	void _update_selection_pattern_from_tilemap_selection();
	void _update_selection_pattern_from_tileset_tiles_selection();
	void _update_selection_pattern_from_tileset_pattern_selection();
	void _update_tileset_selection_from_selection_pattern();
	void _update_fix_selected_and_hovered();
	void _fix_invalid_tiles_in_tile_map_selection();

	void patterns_item_list_empty_clicked(const Vector2 &p_pos, MouseButton p_mouse_button_index);

	///// Bottom panel common ////
	void _tab_changed();

	///// Bottom panel tiles ////
	VBoxContainer *tiles_bottom_panel = nullptr;
	Label *missing_source_label = nullptr;
	Label *invalid_source_label = nullptr;

	ItemList *sources_list = nullptr;
	MenuButton *source_sort_button = nullptr;

	Ref<Texture2D> missing_atlas_texture_icon;
	void _update_tile_set_sources_list();

	void _update_source_display();

	// Atlas sources.
	TileMapCell hovered_tile;
	TileAtlasView *tile_atlas_view = nullptr;
	HSplitContainer *atlas_sources_split_container = nullptr;

	bool tile_set_dragging_selection = false;
	Vector2i tile_set_drag_start_mouse_pos;

	Control *tile_atlas_control = nullptr;
	void _tile_atlas_control_mouse_exited();
	void _tile_atlas_control_gui_input(const Ref<InputEvent> &p_event);
	void _tile_atlas_control_draw();

	Control *alternative_tiles_control = nullptr;
	void _tile_alternatives_control_draw();
	void _tile_alternatives_control_mouse_exited();
	void _tile_alternatives_control_gui_input(const Ref<InputEvent> &p_event);

	void _update_atlas_view();
	void _set_source_sort(int p_sort);

	// Scenes collection sources.
	ItemList *scene_tiles_list = nullptr;

	void _update_scenes_collection_view();
	void _scene_thumbnail_done(const String &p_path, const Ref<Texture2D> &p_preview, const Ref<Texture2D> &p_small_preview, Variant p_ud);
	void _scenes_list_multi_selected(int p_index, bool p_selected);
	void _scenes_list_lmb_empty_clicked(const Vector2 &p_pos, MouseButton p_mouse_button_index);

	///// Bottom panel patterns ////
	VBoxContainer *patterns_bottom_panel = nullptr;
	ItemList *patterns_item_list = nullptr;
	Label *patterns_help_label = nullptr;
	void _patterns_item_list_gui_input(const Ref<InputEvent> &p_event);
	void _pattern_preview_done(Ref<TileMapPattern> p_pattern, Ref<Texture2D> p_texture);
	bool select_last_pattern = false;
	void _update_patterns_list();

	// General
	void _update_theme();

	// Update callback
	virtual void tile_set_changed() override;

protected:
	static void _bind_methods();

public:
	virtual Vector<TabData> get_tabs() const override;
	virtual bool forward_canvas_gui_input(const Ref<InputEvent> &p_event) override;
	virtual void forward_canvas_draw_over_viewport(Control *p_overlay) override;

	TileMapEditorTilesPlugin();
	~TileMapEditorTilesPlugin();
};

class TileMapEditorTerrainsPlugin : public TileMapEditorPlugin {
	GDCLASS(TileMapEditorTerrainsPlugin, TileMapEditorPlugin);

private:
	UndoRedo *undo_redo = nullptr;
	ObjectID tile_map_id;
	int tile_map_layer = -1;
	virtual void edit(ObjectID p_tile_map_id, int p_tile_map_layer) override;

	// Toolbar.
	HBoxContainer *toolbar = nullptr;

	Ref<ButtonGroup> tool_buttons_group;
	Button *paint_tool_button = nullptr;
	Button *line_tool_button = nullptr;
	Button *rect_tool_button = nullptr;
	Button *bucket_tool_button = nullptr;

	HBoxContainer *tools_settings = nullptr;

	VSeparator *tools_settings_vsep = nullptr;
	Button *picker_button = nullptr;
	Button *erase_button = nullptr;

	VSeparator *tools_settings_vsep_2 = nullptr;
	CheckBox *bucket_contiguous_checkbox = nullptr;
	void _update_toolbar();

	// Main vbox.
	VBoxContainer *main_vbox_container = nullptr;

	// TileMap editing.
	bool has_mouse = false;
	void _mouse_exited_viewport();

	enum DragType {
		DRAG_TYPE_NONE = 0,
		DRAG_TYPE_PAINT,
		DRAG_TYPE_LINE,
		DRAG_TYPE_RECT,
		DRAG_TYPE_BUCKET,
		DRAG_TYPE_PICK,
	};
	DragType drag_type = DRAG_TYPE_NONE;
	bool drag_erasing = false;
	Vector2 drag_start_mouse_pos;
	Vector2 drag_last_mouse_pos;
	HashMap<Vector2i, TileMapCell> drag_modified;

	// Painting
	HashMap<Vector2i, TileMapCell> _draw_terrain_path_or_connect(const Vector<Vector2i> &p_to_paint, int p_terrain_set, int p_terrain, bool p_connect) const;
	HashMap<Vector2i, TileMapCell> _draw_terrain_pattern(const Vector<Vector2i> &p_to_paint, int p_terrain_set, TileSet::TerrainsPattern p_terrains_pattern) const;
	HashMap<Vector2i, TileMapCell> _draw_line(Vector2i p_start_cell, Vector2i p_end_cell, bool p_erase);
	HashMap<Vector2i, TileMapCell> _draw_rect(Vector2i p_start_cell, Vector2i p_end_cell, bool p_erase);
	RBSet<Vector2i> _get_cells_for_bucket_fill(Vector2i p_coords, bool p_contiguous);
	HashMap<Vector2i, TileMapCell> _draw_bucket_fill(Vector2i p_coords, bool p_contiguous, bool p_erase);
	void _stop_dragging();

	enum SelectedType {
		SELECTED_TYPE_CONNECT = 0,
		SELECTED_TYPE_PATH,
		SELECTED_TYPE_PATTERN,
	};
	SelectedType selected_type;
	int selected_terrain_set = -1;
	int selected_terrain = -1;
	TileSet::TerrainsPattern selected_terrains_pattern;
	void _update_selection();

	// Bottom panel.
	Tree *terrains_tree = nullptr;
	ItemList *terrains_tile_list = nullptr;

	// Cache.
	LocalVector<LocalVector<RBSet<TileSet::TerrainsPattern>>> per_terrain_terrains_patterns;

	// Update functions.
	void _update_terrains_cache();
	void _update_terrains_tree();
	void _update_tiles_list();
	void _update_theme();

	// Update callback
	virtual void tile_set_changed() override;

public:
	virtual Vector<TabData> get_tabs() const override;
	virtual bool forward_canvas_gui_input(const Ref<InputEvent> &p_event) override;
	virtual void forward_canvas_draw_over_viewport(Control *p_overlay) override;

	TileMapEditorTerrainsPlugin();
	~TileMapEditorTerrainsPlugin();
};

class TileMapEditor : public VBoxContainer {
	GDCLASS(TileMapEditor, VBoxContainer);

private:
	UndoRedo *undo_redo = nullptr;
	bool tileset_changed_needs_update = false;
	ObjectID tile_map_id;
	int tile_map_layer = -1;

	// Vector to keep plugins.
	Vector<TileMapEditorPlugin *> tile_map_editor_plugins;

	// Toolbar.
	HBoxContainer *tile_map_toolbar = nullptr;

	PopupMenu *layers_selection_popup = nullptr;
	Button *layers_selection_button = nullptr;
	Button *toogle_highlight_selected_layer_button = nullptr;
	void _layers_selection_button_draw();
	void _layers_selection_button_pressed();
	void _layers_selection_id_pressed(int p_id);

	Button *toggle_grid_button = nullptr;
	void _on_grid_toggled(bool p_pressed);

	MenuButton *advanced_menu_button = nullptr;
	void _advanced_menu_button_id_pressed(int p_id);

	// Bottom panel.
	Label *missing_tileset_label = nullptr;
	TabBar *tabs_bar = nullptr;
	LocalVector<TileMapEditorPlugin::TabData> tabs_data;
	LocalVector<TileMapEditorPlugin *> tabs_plugins;
	void _update_bottom_panel();

	// TileMap.
	Ref<Texture2D> missing_tile_texture;
	Ref<Texture2D> warning_pattern_texture;

	// CallBack.
	void _tile_map_changed();
	void _tab_changed(int p_tab_changed);

	// Updates.
	void _layers_select_next_or_previous(bool p_next);
	void _update_layers_selection();

	// Inspector undo/redo callback.
	void _move_tile_map_array_element(Object *p_undo_redo, Object *p_edited, String p_array_prefix, int p_from_index, int p_to_pos);

protected:
	void _notification(int p_what);
	void _draw_shape(Control *p_control, Rect2 p_region, TileSet::TileShape p_shape, TileSet::TileOffsetAxis p_offset_axis, Color p_color);

public:
	bool forward_canvas_gui_input(const Ref<InputEvent> &p_event);
	void forward_canvas_draw_over_viewport(Control *p_overlay);

	void edit(TileMap *p_tile_map);

	TileMapEditor();
	~TileMapEditor();

	// Static functions.
	static Vector<Vector2i> get_line(TileMap *p_tile_map, Vector2i p_from_cell, Vector2i p_to_cell);
};

#endif // TILE_MAP_EDITOR_PLUGIN_H
