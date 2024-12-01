// TODO: get hits_actors, is_platform, is_actor worked out
// (intimately tied to collision, at least eventually)
// these things may become a bit field

// TODO: how to keep the head from flying up forever?
// (has to be fixgured out; but doesn't really impact gameplay)

// NOTE: there may be a cool editor that looks kinda like sheet music
// our current level would look something like this
// 0  1  2  3  4  5  6  7
// 1     1    X1    X1
//          2          X2

// LATER: telegraph where things will spawn?
// LATER: draw a metronome
// LATER: both hands coming from sides

// multiple phases
// like juggling
// todo actually learn siteswap
// todo all events must be driven off the same clock
// (FORNOW: don't make them reactive--the pushing things away layer fine, but may go away)
// -- player shouldn't have to guess how many shots are necessary to kill something?
// ! the hand can be killed by time, not by your shots (shots just delay it)
// !! same with head

// XXX: damage -> age
// XXX: timing off of level->frame_index
// XXX: better way of creating things with logic (update group actually seems good)
// XXX: handle bullet age as just getting far away
// XXX: any way of handling child objects


/* Thing *hand = Acquire_Or_Recover_Slot__SETS_PERSISTENT_TO_TRUE(); */
/* Thing *hand2 = Acquire_Or_Recover_Slot__SETS_PERSISTENT_TO_TRUE(); */
/* int &numberOfHandKills             = Integer__MUST_BE_SAME_ORDER_EVERY_TIME(); */
/* int &numberOfFramesSinceHandKilled = Integer__MUST_BE_SAME_ORDER_EVERY_TIME(); */
// FORNOW: 4-4 time
// https://soundcloud.com/soulon10/deep-beats-1-jungle-and-footwork-mix-2023-edition-160-bpm
// LATER: redo with real's

// LATER: lucy and miao should fire at the rate of the music

