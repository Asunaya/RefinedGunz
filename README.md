# RefinedGunz

Requires Visual Studio 2015 to compile

####Features (lots unfinished):

- 3 netcode modes:
 - Server-based -- only enabled if game_dir in server.xml points to a valid game directory that contains animation and map files
 - Peer to Peer Antilead -- Peer to peer, attacker does immediate hit registration
 - Peer to Peer Lead -- Peer to peer, target does immediate hit registration
- Ingame voice chat
- Passwords hashed on the client with BLAKE2b, and encrypted on the server with scrypt
- Remade chat -- resizable, movable, copyable text, unlimited history, transparent background
- Replays:
 - Runs multiple formats -- have so far tested Official V4, V6, and V11; Freestyle Gunz V7 (both ABIs), V8 and V9; Dark Gunz V6
 - Seek bar to jump to different time locations
 - Stats in the replay list -- shows gametype, map name, stage name, player names and scores
- MatchServer supports both SQLite and MSSQL
- MatchServer comprises all the previous modules (MatchAgent, Locator) by default, although you can run them freestanding as well
- Timescale -- /timescale \<ratio> when starting the game with the /game \<map> command-line option
- Registration at the login screen
- Borderless fullscreen
- OC/UC fixed (untested)
- Basic info update rate increased from 10 to 50
- Spectator mode
- Gladiator changes:
 - Invulnerability states removed
 - Massive range weirdness fixed
 - Reduced massive damage from the distance-scaling multiplier [0.9, 3.0] to [0.9, 1.5]
 - Recoil randomness at lower ping when target registers the hit fixed
 - Ground massive animation speed reduced to 90%
