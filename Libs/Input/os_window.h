#pragma once
#include <map>
#include <unordered_map>
#include <functional>
#ifndef __APPLE__
#include <vulkan.hpp>
#endif
#include <boost/utility.hpp>

#include "handler.h"

namespace Phyre {
namespace Input {

class Window : boost::noncopyable {
public:

    // -------------------------- Construction/Destruction -------------------
    /**
    * \param width The width of the window
    * \param height The height of the window
    * \param title The title of the window
    */
    Window(int width, int height, const std::string& title);
    ~Window();

    // --------------------------- Interface ---------------------------------

#ifndef __APPLE__
    /**
     * \param instance The vulkan instance required to create the vulkan surface
     * \return A vulkan surface from the OS window
     */
    vk::SurfaceKHR CreateVulkanSurfaceKHR(const vk::Instance& instance) const;
#endif

    /**
     * \brief Close this window
     */
    void Close();

    /**
     * \brief Bind a handler to us. Handlers are automatically unbound when their lifetime expires.
     * \param handler The pointer to the handler we wish to dispatch events to
     */
    void Bind(Handler::Pointer handler) const;

    /**
     * \brief Update the cursor mode for this window
     * \param mode The cursor mode we will be using
     */
    void SetCursorMode(Cursor::Mode mode) const;

    /**
     * \return The cursor mode this window is currently in
     */
    Cursor::Mode CursorMode() const;

    /**
     * \return The position of the cursor in this window
     */
    glm::vec2 CursorPosition() const;
    void SetCursorPosition(const glm::vec2& coordinates) const;

    /**
    * \param mouse_button The mouse button we wish to query state from
    * \return The state of the mouse button (i.e. pressed or released)
    */
    Action MouseButton(Mouse mouse_button) const;

    /**
    * \return Returns true if the window updated successfully.
    */
    bool Update() const;

private:
    // -------------------------- Type Definitions -------------------------
    typedef GLFWwindow OSWindow;
    typedef std::multimap<Handler::Priority, Handler::Weak> HandlerMap;
    typedef std::unordered_map<Mouse, Action> MouseActionMap;

    // --------------------------- Initializers ------------------------------

    void InitializeMouseActionMap();

    // ---------------------- Event Dispatching ----------------------------

    // Dispatch events to all bound handlers
    void DispatchEvents();

    /**
     * \brief Manage a handler
     * \param handler The handler we want to manage
     */
    void Add(Handler::Pointer handler) const;

    // Get the set of handlers we are managing
    std::shared_ptr<HandlerMap> handlers() const { return p_handlers_; }

    // ------------------------ OS Window Callbacks ------------------------
    static void OSFramebufferResizeCallback(OSWindow* p_os_window, int width, int height);
    static void OSWindowMousePositionCallback(OSWindow* p_os_window, double x, double y);
    static void OSWindowKeyCallback(OSWindow* p_os_window, int key, int /*scancode*/, int action, int mods);
    static void OSMouseButtonCallback(OSWindow* p_os_window, int button, int action, int mods);
    static void OSMouseScrollCallback(OSWindow* p_os_window, double x_offset, double y_offset);
    static void OSWindowCloseCallback(OSWindow* p_os_window);

    // --------------------------- Stateless Helpers -----------------------
    typedef std::function<void(Handler::Pointer)> NotificationCallback;
    static void NotifyHandlers(OSWindow* p_os_window, NotificationCallback callback);
    static void UpdateMouseButton(OSWindow* p_os_window, int button, int action);

    // -------------------------- Data Members -----------------------------
    // A pointer to the underlying OS Window
    OSWindow* p_os_window_;

    // The handlers to whom we dispatch events to
    // This is a shared_ptr because one of the handlers may end up deleting this object
    // so it is important to keep a shared_ptr to it to prolong lifetime
    std::shared_ptr<HandlerMap> p_handlers_;

    // Maintains the state of the mouse buttons
    MouseActionMap mouse_action_map_;

    // -------------------------- Logging Helper ---------------------------
    static const std::string kWho;
};
}
}
