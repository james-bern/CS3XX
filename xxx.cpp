
// XXX: restore shift held 15deg snap functionality
// XXX: restore spoofing api
// XXX: 0 (ability to just click / click and drag to set color)
// XXX: consuming a bunch of events in one frame broken again
// XXX  until ctrl+o is processed, we can't know that the next event should be a GUI event
// XXX: we need actually a whole nother layer and intermediate rep that happens preclassification
// XXX: RAW_USER_EVENT_TYPE_KEY_PRESS
// XXX: RAW_USER_EVENT_TYPE_MOUSE_PRESS
// XXX: restore back undo/redo
// XXX: muratori type "language" for scripts
// XXX: restore color clicking
// XXX: restore SQ[0-9] functionality
// XXX: undo redo
// XXX: figuring out what kind of an event an interaction is needs to happen in the callback layer
// XXX: problem with spoof in how you're handling snapping snap only getting called in the real version maybe let's just snap later?--i don't really like this
// XXX: good end to end test
// XXX hotkey not recognized message should print string equivalent of what the hotkey was
// XXX: switch entities over to taking vec2's (compression too important)
// XXX: restore hot pane persistence when dragging (so 3D camera doesn't move all weird)
// XXX: restore click and drag selection functionality
// XXX: restore ability to move cameras
// XXX: restore ability to move cameras
// XXX: move aesthetics to an AestheticsState
// XXX: click and drag in the text boxes
// XXX: what would a demo app look like that proves you've actually gotten rid of all the /2 business (would need scissoring)
// ???: what about when popup becomes empty?--need some easy way of saying is zero or whatever, num_cells is worrisome
// XXX: weird factor of 2-ness with mouse
// XXX: get rid of all factors of 2
// XXX: ? get rid of gui_NDC_from_screen versus NDC_from_screen
// XXX: cursor in popup
// XXX: fleury ops
// XXX: bring offset plane back
// XXX: move away from gui_printf for popups (bring cow into the app)
// XXX cirle diameter goes back to zero for some reason after making a couple circles and pressing enter
// XXX: the USER_EVENT_TYPE_GUI_MOUSE should be exactly what is ready to be consumed by the popup (it's just popup food) -- dragging comes later 
