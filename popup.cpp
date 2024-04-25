// TODO: return type of this tells you what kind of special event to send (should just return an event)
// TODO: mouse2D event spoof -> constructor (separate construction of the event from enqueueing)
//
// CTRL+O O M A X . D X F ENTER S A Y E 5 0 ENTER ...
//                          ^
//                          |
//                          are we *replacing* this event in the queue (with 0-n other events?)
//                          or, do we just record everything that happened (nothing special about these events)
//
// ???: how do the popups interact with our current history system
// TODO: review how the history system works (on paper)


// conversation works like dis:
// - we have a bunch of state (forgive me based carmack)
// - (most) user events are recorded
// - expensive world state's are snapshotted
// - to undo a user event, we go back to a snapshot of the state, and play back all the events after that except the last one
// - ez pz
// - subtlety:
// - - some *pre*-user events actually get turned into other user events
// - - pressing Z gets transformed into a MOUSE_2D (0, 0) and we apply and record that instead
// - - snapped mouse presses also get pre-processed and then we apply and record a bare MOUSE_2D
// - - question:
// - - - is this a good idea?
// - - - why not just record everything?
// - - - (small) time savings on undo, redo is not a good reason
// - - - if literally everything except undo and redo happened in the big ol' process user event function, that would be very simple
// - - - THOUGH it still wouldn't answer the question of popups (why not?)
// - - - SIMPLIFICATION: what if we just process a single fresh event per frame (we don't need to hit them all)
// - - - - then we don't have to worry about multi-draw
// - TODO: 0) only call process_fresh_event once per frame
// - TODO: 1) move popup stuff into the giant process_fresh_event (it handles enters just like the console stuff; it should be in the same place) -- it's really just an upgrade on the console stuff
// - LATER: X) ? move snapping into process_fresh_event
// - LATER: X) move Y into process_fresh_event
// - PLAN: move snapping into update
// 
// so:
// - why should the popups be any different


// // TODO: First Pass -- just a list of fields you can tab between
// TODO: circle radius popup
// TODO: pressing COW_KEY_ENTER applies the popup
// // NOTE: this should (sometimes) happen one layer up; a popup COW_KEY_ENTER gets translated into a spoofed MOUSE_2D event (circle radius) which gets sent down
// // NOTE: but sometimes (like with the stuff we have already done that uses the console), it's not clear what moving it up a layer gets us
// should support 'X' for 'X', 'Y' coordinates
// 'E' will use it
// 'F' will use it
// TODO: COW_KEY_TAB to move between fields in the popup
// // TODOlATER: checkbox for flipping
// // TODOLATER: e.g., fillet command remembers previously used fillet radius
// // TODOLATER: e.g., circle popup has radius and diameter and circumference, and popup system knows which one changed and calculates the other ones (WHAT IS THE RIGHT USAGE CODE)
// // MAYBELATER: multiple popup windows

// NOTE: extrude add could also allow for mouse input

struct PopupConfiguration {
}

struct PopupConfiguration {
}

#define POPUP_TYPE_NONE 0 // fillet (
#define POPUP_TYPE_FORWARD_KEY_ENTER 1 // extrude
#define POPUP_TYPE_SPOOF_MOUSE_2D 2
void popup_popup(uint32 type, PopupConfiguration config) {
}

void popup_draw() {
}

// TODO: is this really necessary or can we just get away with the console?
//       is there really much of a difference?
//       - big difference is that the current console system is buried inside of standard_event_process
//         and can't spoof mouse events like it could if it were a layer up
// TODO: 

#if 1 // usage code

// TODO???: can just read it out as needed

popup_popup(POPUP_ENTER_TYPE_NONE, "fillet radius");

popup_popup(POPUP_ENTER_TYPE_FORWARD_ENTER, "extrude length out", "extrude length in");

popup_popup(POPUP_ENTER_TYPE_FORWARD_ENTER, "extrude length up", "extrude length down");

// // // NOTE: everything is already known
// // NOTE: mode is already known
// popup_popup(POPUP_ENTER_TYPE_SPOOF_MOUSE_2D, "circle radius");
// popup_is_up = true;

// popup_popup(POPUP_ENTER_TYPE_SPOOF_MOUSE_2D, "box length", "box width");
popup_is_up = true;


{ // NOTE: this goes in the queue processing layer or similar (should only be called once)
    if (click_mode == CLICK_MODE_CREATE_BOX) {
        if (popup_popup("box_width", popup.param1, "box_length", popup.param2)) {
            spoof_MOUSE_2D_event(first_click_x + popup.param1, first_click_y + popup.param2);
        }
    } else if (click_mode == CLICK_MODE_CREATE_CIRCLE) {
        real32 prev_circle_radius = popup.circle_radius;
        real32 prev_circle_diameter = popup.circle_diameter;
        if (popup_popup("circle_radius", &popup.circle_radius, "circle_diameter", &popup.circle_diameter)) {
            spoof_MOUSE_2D_event(first_click_x + param1, first_click_y);
        } else {
            if (popup.circle_radius != prev_circle_radius) popup.circle_diameter = 2 * popup.circle_radius;
            else if (popup.circle_diameter != prev_circle_diameter) popup.circle_radius = popup.circle_diameter / 2;
        }
    } else if (click_mode == CLICK_MODE_CREATE_FILLET) {
        popup_popup("fillet radius", &popup.fillet_radius);
    } else if (click_modifier == CLICK_MODIFIER_EXACT_X_Y_COORDINATES) { // sus calling this a modifier but okay
        if (popup_popup("x coordinate", "y coordinate")) {
            spoof_MOUSE_2D_event(popup.param1, popup.param2)
        }
    } else {
        popup.param1 = 0;
        popup.param2 = 0;
    }
}

