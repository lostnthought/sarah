# Vixen
### Vixen is a chess engine written in C (and if it's online you can challenge it on [lichess](https://lichess.org/@/vixen_is_very_cool)). 

It didn't start in C, though. It started in Rust, at the start of 2025. It was bad. 

But it always bugged me that such a cool name like "Vixen" went to waste. And that's why I started from scratch. So that I could have a GOOD chess engine with a sick name. 

But can we all just pretend that I wanted to teach others about why chess programming is magical?

Vixen is UCI compliant in the same way that I'm government compliant. I have an ID and I begrudgingly follow the law. Don't expect me to do so with a smile on my face. Vixen's the same way. It's actually not *that* bad, but there is still no parsing for increment and no pondering implemented. Yet.

Anyway, here's the brief summary:
- Pseudolegal move generation
- Polyglot support via zobrist hashing
- Evaluation with pawn hashing
- Search, with all of the typical alpha-beta bells and whistles
- Time control, which is honestly just one if statement

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

So yeah, go challenge it on Lichess if it's online. Any time control. Seriously. To prove how serious I am, we'll have a list here of all of the humans who have beaten Vixen. Hoping to fill it up someday.
-
-
-
-
-

  
And a big thank you to all that listen to me ramble about my nerd stuff all the time. Seriously, thank you.
