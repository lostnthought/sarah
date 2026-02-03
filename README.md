# Sarah
### Sarah is a chess engine written in C (and if it's online you can challenge it on [lichess](https://lichess.org/@/vixen_is_very_cool)). The video I made that runs through its development from start to finish and to share my love of chess programming with the world is [here](https://www.youtube.com/watch?v=QzS8HQ3dVWA).
* Some have asked for old versions of some files, so I have them in old_versions. These are genuinely just for historical purposes. The files themselves are warzones but I tried to upload the ones that represented early ideas for the engine.

It didn't start in C, though. It started in Rust, at the start of 2025. It was bad. 

But it always bugged me that such a cool name like "Vixen" went to waste. And that's why I started from scratch. So that I could have a GOOD chess engine with a sick name. 

But can we all just pretend that I wanted to teach others about why chess programming is magical?

And then I was informed that the name was taken. Earth-shattering, really. I am a peculiar, strange, living-on-the-fringe of society type of person. I simply never consider that someone else would do something remotely similar to what I would do. Well...

Sarah is the name I came up with, and not because it follows the (objectifying? romantic?) tradition of men naming their cars or boats or what have you after women, but because it's also the name of my favorite label, the legendary sarah records. Which you should naturally check out if you aren't familiar. Also within the music tradition there is the dramatic and brooding "what sarah said" by death cab for cutie which I actually think is a good song even though some might hate it. There's also sara bareilles which doesn't have an h but I think she is one of those talented female singer-songwriters to come out of the early 2000s. Really that's where it ends, believe it or not there just aren't that many good songs named after sarah. Sarah records is more than enough for me.

Sarah is UCI compliant in the same way that I'm government compliant. I have an ID and I begrudgingly follow the law. Don't expect me to do so with a smile on my face. Sarah's the same way. It's actually not *that* bad, but there is still no pondering implemented. Yet.

Anyway, here's the brief summary:
- Pseudolegal move generation
- Polyglot support via zobrist hashing
- Handcrafted Evaluation with pawn hashing
- Search(.c) (In order: Delta, QSEE, QChkPrune, RFP, Razoring, NMP, Probcut, IID, Extensions, LMP, Futility, ContPrune, SEE, SE, LMR)
- Texel tuner (SGDM(R) and ADAMW(R)) (tuner.c)
- SPSA tuner (spsa.c)
- Hash key updates for each piece types
- Corrective history for pawns, nonpawns, material, king + major, king + minor, continuation
- Continuation history, capture history, and regular history
- Stat tracking for each heuristic used in search

I did spend a while documenting stuff with all of these little @briefs and whatever. It is a bit of a mess due to the absurd amount of inlining though.

Here's a todo list of the stuff that I'm working on:
- Incrementalizing eval
- Tuning search parameters and margins
- Hand tuning psqts and eval terms further
- Releasing a binary with actual commands and stuff
- King safety influences pruning / reductions

I wanted to give a list here of projects and resources that I directly used in writing my engine:
- https://github.com/yukarichess/
- https://github.com/lithander/MinimalChessEngine
- https://github.com/MichaelB7/Kohai-Chess
- Code Monkey King! https://www.youtube.com/@chessprogramming591
- And of course: https://www.chessprogramming.org

So yeah, go challenge it on Lichess if it's online. 
Any time control. Seriously. To prove how serious I am, we'll have a list here of all of the humans who have beaten Sarah. Hoping to fill it up someday.

-
-
-
-
  
And a big thank you to all that listen to me ramble about my nerd stuff all the time. Seriously, thank you.
