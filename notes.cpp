
// TODO: use STATE, OTHER, POPUP, TWO_CLICK_COMMAND, FEATURE_PLANE (don't go all the way down to bare pointers)
// TODO: save green and red meshes to visualize the removal and addition (this is OTHER crap)

// BUG: SEGFAULT when messing around with pausing (TODO go back and check all the vec3 int3 stuff on mesh)

// TODO: tab completion when inputing save/load filenames
// TODO: saving dxf
// TODO: confirm overwrite saving stl / dxf

// TODO: formalize timestep (support variable display refresh rate)
// XXX: 'P' to pause
// XXX: rename STATE, OTHER
// XXX: rename NDC_from_Screen -> NDC_from_Pixel
// XXX: rename Identity
// XXX: only use C++-style casts 
// XXX: switch to regular old bool
// XXX: mesh -> vec3's

// XXX: enums
// XXX: color for messagef
// XXX: simplify selected triangles color (just use JUICEIT_EASYTWEEN)

// TODO: select connected broken on 10mm filleted 50mm box
// TODO: restore checkpoint one before on extrudes?

// TODO: something is slow now (3d picking

// TODO: overwrite the tween (bool?) when clicking the mouse

// TODO: fix up the dragging of the mouse while grabbing the popup

// XXX: color -> color_code
// TODO: programmatic toggle on Sleep


// TODO: update help

// XXX: selection timer on dxf entities so they can flash white
// TODOLATER: entities transparency appear and disappear better

// TODO: printing program to console (storing program as a string)
// TODO: drag and drop of program
// TODO: how are we going to distribute files

// TODO: cow upgrades

// TODO: follow 2-3 hours of autocad tutorials


// TODO: floating sketch plane should tween in size
// TODO: initial plane should be scaled to match the size of the selected face 

//////////////////////////////////////////////////
// DEBUG /////////////////////////////////////////
//////////////////////////////////////////////////

// #define DEBUG_HISTORY_DISABLE_HISTORY_ENTIRELY
// #define DEBUG_HISTORY_DISABLE_SNAPSHOTTING
// #define DEBUG_DISABLE_EASY_TWEEN

//////////////////////////////////////////////////
// NOTES /////////////////////////////////////////
//////////////////////////////////////////////////

// // things we are always unwilling to compromise on
// frame-perfect UI / graphics (except first frame FORNOW)
// ability to churn through a bunch of events in a single frame (either by user interacting really fast, spoofing, or undo/redo-ing)

// // things we are almost always unwilling to compromise on
// zero is initialization
// keep the tool (line_entity, circle, box, move, ...) code simple, formulaic, and not-clever
// - repetition is fine
// we write things ourselves
// - the primary reason to do this is "we can do better"
// - also it's the only way to really understand the system

// // things i feel weird/uncomfy about
// i have a type variable and substructs in Entity and Event so i don't get confused; ryan fleury doesn't like this but i sure seem to (shrug)
// i have some repetition (grug say hmmm...) HotkeyUserEvent and GUIKeyUserEvent are the same thing... popup MOVE and popup LINE are the same thing...

// // misc
// if order doesn't matter, go with alphabetical
// one reason to use consistent variable names is for searchability (a for alpha and for first point in a line is suboptimal)
//                                                                  instead, use alpha and point_one point_two or something
//                                                                  (r for red and radius is suboptimal)
// we're often willing to trade speed and our love of pure-C for readability
// for example, using vec2 and overloading the + operator makes code way easier to read

// // thing we aspire for Conversation to be
// "juicy"
// - watch this: https://www.youtube.com/watch?v=Fy0aCDmgnxg
// - watch this: https://www.youtube.com/watch?v=AJdEqssNZ-U

// // vocab
// an entity is a line_entity or arc_entity in a dxf


// BUG: clicking with sc doesn't do anything sometimes
// BUG: sw broken for very short arcs of huge circles


// // TODO: jim tasks
// cow
// TODO: better text_draw
// TODO: Consolas font
// TODO: upgrade soup_draw and eso_*; redocument
// TODO: upgrade basic_draw
// TODO: gui printf that doesn't automatically add a newline
// TODO: check whether everything broken on windows
// other
// XXX: " " that just replicates LAYOUT's funcionality
// XXX: "SHIFT+ " could be for enter mode
// TODO#define DEBUG_HISTORY_RECORD_EVERYTHING
// TODO#define DEBUG_HISTORY_CHECKPOINT_EVERYTHING
// TODO: remove all usage of gui-printf (after folding in cow)
// NOTE multiple popups is a really easy situation to find self in (start drawing a line_entity then extrude; this seems like a job for much later
// XXX: dragging hot pane divider
// TODO: bring cameras into app
// TODO: possible bug: GUI_MOUSE getting triggered when it shouldn't
// // TODO: consider recording an event based on whether anything in the state changed
//          (this would remove the need to do all the dirty checks)
//          and you would only need to do it for new events
// NOTE: this is a good idea
// the recursive crap in _standard_event_process_NOTE_RECURSIVE you're doing now could be a problem
// (another layer atop it would probably do the trick -- _original_call_standard_event_process)
// TODO: ghostly section view on 2D side
// TODOLATER repeated keys shouldn't be recorded (unless enter mode changes) -- enter mode lambda in process
// TODOLATER: don't record saving
// NONO: switch key_lambda over to strings like the scripts "a" "A" "^a" "^A"
// TODO if (awaiting_second_click) // BAD VERY BAD NOW
// TODO: really formalize char_equivalent
// XXX: figure out system for space bar
// TODO: merge a copy of cow into the app (split off into own repo with a cow inside it)



// // TODO: good undergrad tasks
// TODO: fast picking (something like a BVH)--need arena (mesh should have its own arena)
// TODO: popup_popup real popup automatically parse formulas
// TODO: ROTATE
// TODO: COPY
// TODO: SCALE
// TODO: OFFSET
// TODO: shell (OFFSET CONNECTED)
// TODO: FILLET CONNECTED (super fillet 'F')
// TODO: perpendicular snap
// TODO: quad snap
// TODO: LAYOUT polygon
// TODO: add LAYOUT's two click mirror
// TODO: being able to type equations into boxes
// TODO: holding shift to make box a square
// TODO: ERASE (including drawing the eraser circle in pixel coordinates
// TODO: three click circle
// TODO: elipse
// TODO: fillet line_entity-arc_entity
// TODO: fillet arc_entity-arc_entity
// TODO: power intersect
// TODO: 1 click intersect
// TODO: 2 click intersect
// TODO: power intersect
// NONO: colors for messagef (warning, error, info, success)
// // TODO: toolbox you can click on that copies over a sketch (like the counter sunk nuts)

// // TODO: big tassks
// TODO: somehow, this should be parsed out of the Drawing
// TODO: ability to scroll through toolbox with mouse



///////////
// POPUP //
///////////

// TODO: the flow here is a bit tortuous
// on the previous frame we store information about the hover state (is the user hovering?--over which cell?--what new cursor position?)
// then...when the user clicks (in the click callback) we can generate a EVENT_TYPE_GUI_MOUSE_PRESS of the appropraite type, which gets processed by the next pass through the popup
// this seems generally fine, i guess what feels weird is that we need to retain some state / store some data (?? perhaps this is morally speaking ScreenState_ChangesToThisDoNOTNeedToBeRecorded data) in order to classify the event--but i suppose this makes sense

// there is a parallel with deciding what to do with key input
// NOTE: this could be split into two event types

// cool text animations when tabbing
// TODO: 'X'
// TODO: mouse click
// TODO: mouse drag
// TODO: paste from os clipboard
// TODO: parsing math formulas
// TODO: field type

// TODO: make this a function
