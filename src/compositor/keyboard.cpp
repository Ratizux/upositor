#include <upositor/compositor/keyboard.hpp>
#include <upositor/utils/container_of.hpp>

Keyboard::Keyboard(Server *parent, wlr_input_device *device)
{
	this->server = parent;
	auto wlroots_keyboard = wlr_keyboard_from_input_device(device);
	this->wlroots_keyboard = wlroots_keyboard;

	struct xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
	struct xkb_keymap *keymap = xkb_keymap_new_from_names(context, NULL,
		XKB_KEYMAP_COMPILE_NO_FLAGS);
	wlr_keyboard_set_keymap(wlroots_keyboard, keymap);
	xkb_keymap_unref(keymap);
	xkb_context_unref(context);

	wlr_keyboard_set_repeat_info(wlroots_keyboard, 25, 600);
	wlr_seat_set_keyboard(server->seat, wlroots_keyboard);

	key_listener.notify = [](wl_listener *listener, void *data)
	{
		// normal key
		auto event=static_cast<wlr_keyboard_key_event*>(data);
		Keyboard *self = container_of(listener, Keyboard, key_listener);
		auto &seat = self->server->seat;
		wlr_seat_set_keyboard(seat, self->wlroots_keyboard);
		wlr_seat_keyboard_notify_key(seat, event->time_msec, event->keycode, event->state);
	};
	wl_signal_add(&wlroots_keyboard->events.key, &key_listener);

	modifiers_listener.notify = [](wl_listener *listener, void *data)
	{
		Keyboard *self = container_of(listener, Keyboard, modifiers_listener);
		auto &seat = self->server->seat;
		auto &wlr_kbd = self->wlroots_keyboard;
		wlr_seat_set_keyboard(seat, wlr_kbd);
		wlr_seat_keyboard_notify_modifiers(seat, &wlr_kbd->modifiers);
	};
	wl_signal_add(&wlroots_keyboard->events.modifiers, &modifiers_listener);
}
