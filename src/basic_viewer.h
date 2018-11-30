/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of EasyGUI: software for processing and rendering
*   meshes and point clouds.
*
*	EasyGUI is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License Version 3
*	as published by the Free Software Foundation.
*
*	EasyGUI is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _EASY3D_BASIC_VIEWER_H_
#define _EASY3D_BASIC_VIEWER_H_

#include <string>
#include <vector>

/*  EasyGUI is really easy to use. That's why it has such a name.
*
*   Just create an instance of BasicViewer, or derive your viewer 
*   from it by re-implementing its draw() function (and other 
*   functions if necessary), and run your viewer:
*		MyViewer viewer("Example");
*       viewer.run();
*/


class  PointCloud;
class  SurfaceMesh;
class  Camera;
struct GLFWwindow;

class BasicViewer
{
public:
	BasicViewer(
		const std::string& title = "BasicViewer",
		int samples = 4,
		int gl_major = 3,
		int gl_minor = 2,
		bool full_screen = false, 
		bool resizable = true, 
		int depth_bits = 24, 
		int stencil_bits = 8
	);

    virtual ~BasicViewer();

	// Enter the application main loop.
	void run();

	//-----------------------------------------------------

	void set_title(const std::string &title);
	const std::string& title() const { return title_; }

	// Set whether or not the viewer is currently visible (assuming all parents are visible)
	void set_visible(bool visible);
	// Return whether or not the viewer is currently visible (assuming all parents are visible)
	bool is_visible() const { return visible_; }

	// Returned values are(0, 0, screen_width, screen_height), so that the origin is
	// located in the lower left corner of the window (OpenGL style coordinate system).
	void  get_viewport(int viewport[4]) const;

	const float* background_color() const { return &background_color_[0]; }
	void set_background_color(float r, float g, float b);
	
	/// Return the actual samples of the viewer
	int	 samples() const { return samples_; }

	/// Update the rendering
	void repaint();

    // explicitly set window size
    void resize(int w,int h);

	// open/save file
	void open_file();
	void save_file();

protected:

	/// Put any initialization that requires a working rendering context 
	/// (e.g., creating OpenGL resources like shaders, textures).
	/// This function will be called before the draw procedure.
	virtual void init() { /* To be overridden */ }

	// Returns **true** if action should be cancelled.
	virtual bool pre_draw() { return false; /* To be overridden */ }

	/// Put your OpenGL draw calls here
	virtual void draw() { /* To be overridden */ }

	// Returns **true** if action should be cancelled.
	virtual bool post_draw() { return false; /* To be overridden */ }

    // external resize due to user interaction
    virtual void post_resize(int w, int h) { /* To be overridden */ }

	/// Put resource collection calls here (e.g., destroy shaders, textures). 
	/// This function is called before the rendering context is destroyed.
	/// NOTE: Avoid collecting OpenGL resources in a destructor because it
	///       might be called after the rendering context is destroyed.
	virtual void cleanup() { /* To be overridden */ }

	/// Mouse button press event handler
	virtual bool mouse_press_event(int x, int y, int button, int modifiers);
	/// Mouse button release event handler
	virtual bool mouse_release_event(int x, int y, int button, int modifiers);
	/// Mouse drag (i.e., a mouse button was pressed) event handler
	virtual bool mouse_drag_event(int x, int y, int dx, int dy, int button, int modifiers);
	/// Mouse free move (i.e., no mouse button was pressed) event handler
	virtual bool mouse_free_move_event(int x, int y, int dx, int dy, int modifiers);
	/// Mouse scroll event handler
	virtual bool mouse_scroll_event(int x, int y, int dx, int dy);

	/// Text input event handler: codepoint is native endian UTF-32 format
	/// NOTE: This one reveals the actual character being sent (not just the physical key)
	virtual bool char_input_event(unsigned int codepoint);
	/// Keyboard event handler. 
	/// NOTE: This function does not reveal the actual character.
	virtual bool key_press_event(int key, int modifiers);
	virtual bool key_release_event(int key, int modifiers);

	/// Handle a file drop event
	virtual bool drop_event(const std::vector<std::string> & /* filenames */) { return false; /* To be overridden */ }

	/// Handle a focus change event
	virtual bool focus_event(bool focused);

private:

	/// Draw the viewer contents
	virtual void draw_all();

	/// Destroy the window
	void destroy();

	virtual void setup_callbacks();

	/* Event handlers. Client code should not touch these */
	virtual bool callback_event_cursor_pos(double x, double y);
	virtual bool callback_event_mouse_button(int button, int action, int modifiers);
	virtual bool callback_event_keyboard(int key, int action, int mods);
	virtual bool callback_event_character(unsigned int codepoint);
	virtual bool callback_event_drop(int count, const char **filenames);
	virtual bool callback_event_scroll(double dx, double dy);
    virtual void callback_event_resize(int w, int h);

protected:
	GLFWwindow*		window_;
	std::string		title_;

	SurfaceMesh*	surface_;
	PointCloud*		pointcloud_;

	int		samples_;	// the actual samples available

	bool	full_screen_;
	bool	visible_;
	float	background_color_[3];

	// enable/disable event processing
	bool	process_events_;

	Camera* camera_;

	// mouse
	int		button_;		// for mouse drag
	int		modifiers_;		// for mouse drag
	bool	drag_active_;	// for mouse drag
	int		mouse_x_;		// for mouse button and scroll events
	int		mouse_y_;		// for mouse button and scroll events
};

#endif	// _EASY3D_BASIC_VIEWER_H_
