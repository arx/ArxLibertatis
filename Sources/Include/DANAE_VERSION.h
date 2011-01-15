/*
===========================================================================
ARX FATALIS GPL Source Code
Copyright (C) 1999-2010 Arkane Studios SA, a ZeniMax Media company.

This file is part of the Arx Fatalis GPL Source Code ('Arx Fatalis Source Code'). 

Arx Fatalis Source Code is free software: you can redistribute it and/or modify it under the terms of the GNU General Public 
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Arx Fatalis Source Code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied 
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Arx Fatalis Source Code.  If not, see 
<http://www.gnu.org/licenses/>.

In addition, the Arx Fatalis Source Code is also subject to certain additional terms. You should have received a copy of these 
additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Arx 
Fatalis Source Code. If not, please request a copy in writing from Arkane Studios at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing Arkane Studios, c/o 
ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
===========================================================================
*/
#define DANAE_VERSION 1.864f
//#define NOEDITOR
//#define ASKPASS
//#define BLUEBYTE
//#define ETRANGELIB
//#define ASKPASS
#ifdef ETRANGELIB
#define ASKPASS
#endif

//////////////////////////////////////////////////////////////////////////////
// TO REMEMBER:
//-------------
// DON'T FORGET TO RESTORE MANA !!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//	-> NEED TO Save Attractors...
//	-> NEED TO Save Cine ... or not ?
//  -> ICE SPELLS -> Kill the interface !
//DONE------------------------------------------------------------------------
//	-> NEED TO Save durability & Poisonous & Poisonous_count & Poisonned...
//  -> NEED TO Save SETSECRET info
//	-> NEED TO Compute REAL physical cylinder when used...
//  -> NEED TO Save UsePath DATA !!!
//	(size modifier isn't taken into account in every routine...)
//NOT NEEDED ANYMORE----------------------------------------------------------
//  -> NEED TO Manage ADD_GOLD to others than player
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// DANAE Versions History													//
//////////////////////////////////////////////////////////////////////////////
// 1.838-839			Near TRUE DEMO Version !!!
// 1.818-1.820+			Portals version
// 1.817	24/01/2002	New external version
// 1.816	22/01/2002	Added MAGIC ON/OFF script command
//						Added MAPMARKER script command
//						Fixed Dead NPC setweapon on reload
// 1.815	21/01/2002	Fixed Some Buggy Poly tests
//						Fixed Object Throwing
//						Fixed Arrow Launching
// 1.814	18/01/2002	Somewhere After Beta III
// 1.811	15/01/2002	Somewhere Near Beta III
// 1.808	11/01/2002	Arf...
// 1.797	05/12/2001	Added SETDURABILITY -C
//						New minimap system
// 1.795	04/12/2001	Demo ??
// 1.793	03/12/2001	Demo ?
// 1.792	30/11/2001	Somewhere Near Demo...
// 1.791	30/11/2001
// 1.790	29/11/2001
// 1.789	28/11/2001	Added ^POISONOUS and corrected ^POISONED
// 1.788	28/11/2001
// 1.787	27/11/2001
// 1.786	26/11/2001	Added Repair
// 1.785	25/11/2001	Fixed many changelevel bugs
// 1.784	23/11/2001	Added ^Poisoned
// 1.783	19/11/2001	Fixed Attractors bug
// 1.781	19/11/2001
// 1.780	19/11/2001
// 1.779	19/11/2001	Corrected Invalid player skills
//						Fixed a bit cylinder move (not really precise on Y values)
//						Fixed Globals not restored before init on Firstload
//						Modified NPC Combat Behavior
//						Added NPC check for Strafe/move_backward moves...
// 1.778	19/11/2001	Fixed fight run/walk combat dist
//						Fixed Buggy Torch Management
//						Fixed a bit Frustrum
//						Fixed small pricing bugs
//						Prices are now evaluated depending on durability/max_durability
//						Cursor becomes interactive when flying over burning torch !
// 1.777	19/11/2001	Fixed misc points
//						CEDRIC version
// 1.776	18/11/2001	Fixed SETTARGET -N bugS
//						Added "Demo" commandline to set demo version on.
//						Fixed Changelevel PopIO popup error message
//						Fixed FLEEEND loop bug
//						Fixed Same Anchor Pathwait bug
//						Fixed Behavior/SetTarget NONE bugs
// 1.775	18/11/2001	Added 2-pass anchor-gen for improved Precision.
//						Fixed Crouch bug (not getting up when necessary).
// 1.774	17/11/2001	Changed (Improved ?) Anchor Generation Method
//						Fixed Two IO release Crashes (SecondaryInventory & DRAGINTER)
// 1.773	16/11/2001	New FishTank Version.
// 1.772	15/11/2001	Best wishes.
// 1.771	15/11/2001	new fixes.
// 1.770	15/11/2001	Fixed Draginter Crash...
// 1.769	15/11/2001	Fishtank Version II (wannabe Version II).
// 1.768	15/11/2001	Fishtank Version.
// 1.765	12/11/2001	Fixed Combat Freeze Bug.
//						Now linked objects become invisible according to main object.
//						Cannot severe members of living NPCs anymore.
//						Fixed Trans-Flat faces crash.
//						Fixed Thrown IO Invalid light computations.
//						Fixed Multiple Spell DynLights incorrectly/not released.
// 1.763	12/11/2001	Removed TransPoly Crash.
//						Corrected Sprites Dependency to Zclip.
//						Released 1.3 Mo of infrequently used Snapshot memory :/
//						Added ChangeLevel Popup (F5).
//						Re-implemented Software Fog, Added D3DFOG checkbox.
// 1.760	08/11/2001	Fixed a bit player movement/jumping/gravity...
//						Added new shop system.
//						Added PLAYER_STACK_SIZE script command.
//						Added SHOP_MULTIPLY script command.
//						Added SHOP_CATEGORY script command.
// 1.757	07/11/2001	Fixed Big Goto loop bug.
// 1.756	07/11/2001	Fixed small bugs (clothes, anchor-gen...).
// 1.755	07/11/2001	Many changes...
// 1.754	05/11/2001  Added SETPLATFORM ON/OFF command.
//						Added SETELEVATOR ON/OFF command.
//						Added ENDGAME command.
//						Added -i flag to TIMER command.
//						Added SETSECRET val/OFF command.
//						Added BOOK Command.
//						Added RIDICULOUS Command.
//						Added NOTE Command.
//						Added SETDURABILITY Command.
//						Added SETSTEPMATERIAL Command.
//						Added SETARMORMATERIAL Command.
//						Added SETWEAPONMATERIAL Command.
//						Added SETSTRIKESPEECH Command.
//						Added YLSIDE_DEATH to SPECIALFX Command.
//						Added BEHIND param to TELEPORT.
//						Added ^demo.
// 1.752	11/10/2001  Last Minute Fixes.
// 1.751	10/10/2001  New Athena Version.
//						Fixed DoubleClick temporarily...
//						Fixed Moulinex.
//						Improved a bit particles efficiency.
// 1.749	10/10/2001  Removed Note Bug.
//						Added Goto/Gosub script index.
// 1.748	10/10/2001  Sound Fixes.
//						New Halo.
//						Interface Fixes.
// 1.747	09/10/2001  Interface.
// 1.746	09/10/2001  New Anchor Gen.
// 1.745	05/10/2001	Ambiances Release Bug.
// 1.744	05/10/2001	++.
// 1.743	04/10/2001	UNICODE.
// 1.738	02/10/2001	Fixed Anchor generation (again...).
//						Fixed clipping when looking up/down.
//						Added gores.
//						Added gore splats on ground/walls.
//						Added Severed members.
//						Added Sparks.
//						Fixed Background polys draw inconsistencies.
//						Fixed EERIE divide/asset failure.
//						Modified a bit Lava Fx.
// 1.731	25/09/2001	Improved Anchors generation.
//						Improved NPC moves precision.
//						Removed patch bug ?
//						Now saves animation list for each io.
//						No more INIT/INITEND msgs sent on reloads.
//						new animated fireballboom.
//						new soundsystem version by Nicolas.
//						Fixed a bit Moulinex.
// 1.719	17/09/2001	Removed KILLME/KILLME loop stack overflow bug
//						Removed buggy Vsync
// 1.712	16/09/2001	Ground Zero ?
// 1.711	16/09/2001	Removed some big bad Memory Leaks in BMP/JPG loaders
// 1.710	15/09/2001	Version Plus Mieux
// 1.708	14/09/2001	Fixed PAK Read...
// 1.707	14/09/2001	Fixed PAK Localisation
// 1.704	13/09/2001	Implemented Command-Line Moulinex
//						Upgraded Save/Load Menu
// 1.703	12/09/2001	Fixed Objects Moulinex Texture Bug
//						"True" Game mode first version
//						Improved CINE command
//						Added ENDINTRO command
// 1.702	11/09/2001	Fixed minor gamesave bug... (invalid var but not used yet)
//						Improved Moulinex (TM)
//						Fixed PNG incrust Threshold
//						Added PRECAST script Command
//						Added -N to TELEPORT Command
//						Quests are now saved with Player Data
//						First Version of Note/book system
// 1.701	10/09/2001	Now restores IOGROUPS !
//						REVIVE now calls Global AND Local Init Initend
//						Added Event GAME_READY
// 1.700	10/09/2001	Removed Patching Bug in C_FONT
//						Added TELEPORT BEHIND func
// 1.699	09/09/2001	Added DAMAGER command
//						Added Damage Type flags to DODAMAGE command
// 1.698	08/09/2001	Added -D flag to HEROSAY (to print text only in debug)
//						Removed "HeadClimb" Bug
//						Improved NPC Sliding.
//						Improved Player Pushing of NPC
//						Saved more data for changelevel/gamesave
//						Added COLLISON_ERROR_DETAIL event
// 1.697	08/09/2001	Added SIDE_L
//						Added ATTRACT Command
//						"Fight" look_at for NPC
//						SetTarget -S
// 1.696	05/09/2001	Added KILLALL option to SPEAK Command.
//						Jump/Slide/Climb
//						Added BEHAVIOR -a FRIENDLY (stare_at)
// 1.695	03/09/2001  Fixed Packer file limit 1024
//						Removed initio stupid info from inventories...
//						Can now skip speeches with space when player controls are off
//						Added -i flag to PLAY command for unique sounds.
// 1.693	30/08/2001  Fixed Physical OFF
// 1.692	30/08/2001	added ^PLAYER_ZONE
//						added ^LAST_SPAWNED
//						added PHYSICAL ON/OFF
// 1.689	27/08/2001	added ON CRITICAL ON COLLIDE_NPC and ON BACKSTAB
//						added SNEAKMODE
//						added AGGRESSION to SETEVENT OFF
//						added TIMER KILL_LOCAL
// 1.688	26/08/2001	added COLLISION_ERROR event.
//						forced Min value for PHYSICAL HEIGHT...
// 1.687	24/08/2001	Gosub Stack is cleared with an ACCEPT or REFUSE
//						Added ^PLAYER_ATTRIB... SKILLS...
//						Added SETNPCSTAT CRITICAL
//						Added SET_TRANSPARENCY %
//						Added ^PLAYER_GOLD
//						Added ADD_GOLD ident val
//						Added ^PRICE ^AMOUNT
//						Added ^ME (contains current IO ident)
//						Fixed PLAYERLOOKAT
// 1.686	24/08/2001	Added SETEVENT DETECTPLAYER OFF
//						Fixed pathfinder called for stuck objects
//						Auto-unstuck/freeze objects on load...
//						Added -U flag to speak (but not applied)
//						Added PLAYANIM -N (no interpolation)
//						ListenerPos now correctly updated.
//						New Athena Version with Reverb/EAX
// 1.685	24/08/2001  bis
// 1.684	23/08/2001	Fixed Weapon Draw/off
//						Added Slope management in physics...
// 1.683	21/08/2001	Fixed GAME Path generation bug...
// 1.682	21/08/2001	Fixes for Cday again
//						WORM mode
//						Seems to have fixed ListenerPos/dir
// 1.681	21/08/2001	Fixes for Cday
// 1.680	21/08/2001	Fixed SetEvent Hear Off
//						Added SETNPCSTAT BACKSTAB_SKILL
//						Added SETNPCSTAT BACKSTAB
//						Added SETNPCSTAT REACH
//						Cday
// 1.679	20/08/2001	VertexLimit in EERIE_3DOBJ raised to 65535...
//						Added SCENE menu
// 1.678	17/08/2001	Set Player Mesh Focal to 210 always (1st Person)
//						Fixed 1st Group hiding bug...
// 1.677	16/08/2001	Broum Brloumpfff Machin PLAYANIM -P Grrrrrrrrrr...
//						Fixed SPEAK -PC !
// 1.676	16/08/2001	Fixed/improved SPEAK -C
//				 		Speeches don't use Timers anymore...
// 1.675	14/08/2001	Many little changes to behavior
//						PLAYANIM -p for remote player control
//						LOADANIM -p to load player anims from another script
//						SPEAK -C !!!
// 1.674	14/08/2001	Restored Fastloads...
//						Modified Clipping...
// 1.673	10/08/2001	Fixed Pathfinding & Anchors ( again ... :( )
//						Can now SETEVENT HEAR ON/OFF.
//						Uses paths uk/fr for speeches
//						Reduced ^objontop sensitivity(+only NPC & ITEMS can be detected now)
//						Added ^BEHAVIOR system variable
//						Added ^INPLAYERINVENTORY system variable
//						Added INVULNERABILITY ON/OFF
//						Restored Inventory System-Played Sound for NPC.
//						Added FLEE_END system-sent event when finished flee path
//						Added SET_DURABILITY command (default 50);
//						Added SET_POISONOUS command
//						Fixed FORCEDEATH bug
//						Fixed a bit Poison Projectile
//						Added MOULINEX (c) (TM) Option
// 1.672bis	09/08/2001	Fixed Light selection bug...
// 1.672	07/08/2001	Some serious improvements to the pathfinder usage.
//						  "   "   "   "  Same for behaviors/Collisions etc...
//						Better Anchor generation system (but still not the best...)
//						Restored weapons... (I hope so)
//			08/08/2001	Fixed WeaponInHand/ WEAPON DRAW bug...
//						Fixed Behavior NONE a bit
//						SPEAK sounds pos now updated in 3D
//			09/08/2001	Now NPCs deals damages as the Player does...
//						Improved a bit arrows
// 1.671	XX/08/2001	Added -K Flag to SPELLCAST command (to kill a previously cast spell)
//						Removed secondary inventories sounds
//						Now Saved Games are Packed :) (but still not compressed)
//						Added SET_FOOD script command
//						Added SET_SPEED script command.
//						Food is now Handled
//						Added First Version of Spell Pre-casting
// 1.670	19/07/2001	Moved some structs from IO to more appropriate places
//						Fixed Textures from Cached Meshes being released...
//						Improved Weapon collision algorithm
//						Implemented first version of Gore
//						Improved a bit code efficiency by removing unneeded indirections
//						Reduced size of some structs.
//						Fixed Threads not correctly released...
//						NPC in WAIT anim are now allowed rotation only with idx 0
//			20/07/2001	Fixed Light Mode Overdraw bug
//						Added a new mouse smoothing... sort of
//						Fixed TELEPORT -a BUG
//						Added -D Flags to Spellcast command (duration)
//						Arrows spawned by player bow are correctly oriented and deals damages
//						Improved a bit physics (Added Rubber param)
//			23/07/2001	Changed/Optimized Anchor structure
//						Fixed INVENTORY ADD objects not being initialized
//						Fixed incorrect ON INIT event sending order in some case...
//						Fixed Replaceme not generating IOIdent.
//			24/07/2001	Removed Anchors number invalid bug
//						Removed 1st person player polys
//						Added ^Price to Script System Variables
//						Removed Thread Kill bug
//			25/07/2001	Moved some more data outside of IO main struct (ITEM data)
// 1.669	07/07/2001	Fixed AltIdx invalid value computing...
//						Moved Pathfind struct to IO_NPCDATA struct
// 1.668	15/07/2001	= Version 1.667 + sound Nico
// 1.667	15/07/2001	Fixed FIX IO tolerance being too low
//						Many things
//						Fixed Zone enter after changelevel
// 1.666	14/07/2001	...
// 1.665	13/07/2001	Added all symbols (Alpha version) for NPC spellcasting
//						Removed Paralyse Player Bug...
//						Improved Quite a Bit IO Movement Management
//						Added First Version of Gold Management
//						Fixed non-NPC IO Trans Bug (Sarco).
// 1.664	13/07/2001	Texture flag NOCOL now managed
//						Added ON STRIKE event
//						Fixed Controlled zone bug ... again ?
// 1.663tris13/07/2001	Fixed côôôôt bug (timer badly initialised on FirstTime Load)
// 1.663bis	13/07/2001	Fixed Spells Crash Bug (particles texture)
// 1.663	13/07/2001	Fixed Global Time Bug after Load/changelevel
//						Fixed Zone Controllers Bug...
// 1.662	13/07/2001	Can now set save path
//						Can now set save name
//						Fixed timer bug
//						Fixed zone bug
//						Removed BIG speech/Timers bug... (lvl14 ALIA bug)
// 1.661	13/07/2001	Fixed FOG freeze bug... :/
//						Improved a bit FOG handling by the way :)
//						ON RELOAD event now sent only for changelevels
//						GLOBAL Vars Correctly Popped Now...
// 1.660	12/07/2001	Can now Save/Load/Changelevel... I hope...
//						Fixed Menu/book Fog
// 1.659	11/07/2001	Fixed Player Invisibility
//						Fixed NPC Levitate Bug (But NPC Levitate still not fully implemented)
//						Fixed Metal Z-Bug
// 1.658	11/07/2001	Fixed some big bugs !!!
// 1.657	10/07/2001	Fixed Ambiance Vol Max <=1
//						Removed Linked Objects Assertion Bug
// 1.656	09/07/2001	New CEdit Version
//						Fixed CROUCH FIGHT modes
//						CROUCH HEIGHT is now managed in physics
//						Added REVIVE script command (-i for init pos)
//						Added ANIM_WALK_SNEAK definition
// 1.655	08/07/2001	Fixed sound stop freeze
//						Fixed Spell Icons bug
//						Added Zone params
//						Added ZONE_PARAM script command
// 1.654	04/07/2001	Restored Z-Map & optimised it a bit
//						Added new zone parameters
//						Fixed 3D backface Removal being too aggressive...
//						Added ^MANA for npc
//						Added RESISTMAGIC RESISTPOISON RESISTFIRE to SETNPCSTAT command
//						Fixed Blood Bug
//						Added SPELLCAST script command...
//						Major changes to Damages engine...
//						Major changes to Spell engine...
//						Updated Spell Engine with Tzu code
//						Fixed Menu Akbaa Symbol Dynlight
//						Fixed Menu Pausing Game
// 1.653	30/06/2001	Behavior Mods
// 1.652	30/06/2001	Fixed Behaviors & NPC movement engine...
// 1.651	29/06/2001	Corrected KeyFrames Bug...
// 1.650	29/06/2001	Plouf
// 1.649	29/06/2001	Added Alternate Animations... (Seems easy said like this...)
// 1.648	28/06/2001	Added Param1 (distance) to ON HEAR event
//						Added ON SUMMONED event (^sender=caster)
//						Added DESTROY command
//						Added ADD_XP command
//						Debugger Upgrade
//						Fixed Reachedtarget NPC Behavior
//						Can now push a bit small NPCs
// 1.647	27/06/2001	Added ON HEAR event
//						Fixed a bit sounds
//						Implemented Stealth Step sounds
// 1.646	27/06/2001	Implemented new transparencies
// 1.645	27/06/2001	Fixed Transparency Bug
//						Can now accelerate conversations with SPACE
//						Fixed IO draw/clipping bug
// 1.644	26/06/2001	Fixed in-book player aspect ratio
//						Fixed in-book runes aspect ratio
//						Removed in-book Bookmarks shadows
//						Fixed in-book 3D objects invalid lights.
// 1.643	26/06/2001	Added PHYSICAL RADIUS/HEIGHT script command
// 1.642	26/06/2001	Corrected CINE Bug
// 1.641	25/06/2001	Removed NPC Highlight
// 1.640	25/06/2001	Behavior MOVE_TO added to debugger
// 1.639	25/06/2001	Highlighted FlyingOverIO
//						Official Debugger Release
// 1.638	25/06/2001  Added ATTACH/DETACH script funcs
//						First version of Script Debugger
//						Added UNSTACKALL to BEHAVIOR command
//						SPEAK now accept [] parameter
//						Fixed PLAY/SPEAK not accepting variable parameters
//						Fixed Tilde bug (not correctly interpreted in LOCAL...)
// 1.637	23/06/2001  Fixed Minor Purge Bug
// 1.636	22/06/2001  Corrected Minor Bug
// 1.635	21/06/2001	Added Purge Option & ForceLoad option in file menu...
//						Improved slightly FPS by optimizing lights management
// 1.634	20/06/2001	Fixed menus (no more spells in menu-mode & pause).
//						Can now resize windowed Danae & content
//						Pixel ratio now constant between various screen modes.
//						Can now launch direct Fullscreen demo.
// 1.633	16/06/2001	"fixed" ? cinematics bug. removed 'V' key for MiniMap Gen
//						Optimized a bit script handling and "IF" statement
// 1.632	16/06/2001	Fixed "endless" Physics Objects
// 1.631	16/06/2001	Now correctly breaks "speaks"
// 1.629	16/06/2001	Removed Memory Leak Bug on Save Game
// 1.628	16/06/2001	Fixed Minimap
// 1.627	16/06/2001	sendevent -g now send to every object in this group
// 1.626	16/06/2001	SET_GROUP accept £var now / Same for SendEvent -g £var
//						Mini-Map
//						Added ON OUCH
//						Added ON COLLIDE_DOOR
// 1.625	15/06/2001	Big Bug
// 1.624	15/06/2001	Merge with nico (Menus Save & Load)
// 1.623	15/06/2001	now restore initpos in saves
// 1.622	15/06/2001	fixes
// 1.621	15/06/2001	Added behaviour stack reset
// 1.620	14/06/2001	Save/Load/change level... need to save IO inventories...
// 1.619	13/06/2001	Save/Load/change level...
// 1.618	12/06/2001	Save/Load/change level...
// 1.617	10/06/2001	Save/Load/change level...
// 1.616	08/06/2001	Save/Load/change level...
// 1.615	06/06/2001	Can now travel between levels... at own risks...
// 1.614	31/05/2001	Some fixing on IO behavior
//						Added NPC/ITEM parameter to SPAWN command
//						Added LIPSYNCH feature, first version
// 1.613	31/05/2001	Added FORCEANGLE script command
//						Added -L Switch to PLAY script command for infinitely looped sounds
// 1.612	29/05/2001	Added ^PLAYERSPELL_XXX system variable.
//						Added QUEST command
//						Added ON SPELLCAST event (param1= spelltextident) ^SENDER=caster
//						Fixed ICE_BLOC bug
// 1.611	28/05/2001	Still some minor fixes
//						Modified AMBIANCE (KILL)
// 1.610	25/05/2001	Added SETSPEAKPITCH %f
//						Added -p (randomize pitch by +/- 10%) to PLAY script command
//						Fixed minor Physics Bug
// 1.609	16/05/2001	Added ^RND_XX system variable to scripting language
//						Added ^SPEAKING system variable to scripting language
//						Minor Fixes & Updates
//						Added Search All Scripts Button
// 1.608	15/05/2001	Fixed Ambiance Bug
// 1.607	15/05/2001	Fixed Ambiance Bug
//						Fixed Small Bugs
// 1.606	15/05/2001	Added Fix Collisions
//						Added many features
//						Merged with Didier
//						Merged with Nicolas
// 1.605	14/05/2001	One bugfix (Pathfinder init)
// 1.604	14/05/2001	Some bugfixes (Pathfinding)
// 1.603	14/05/2001	Many bugfixes
// 1.602	13/05/2001	Fixed Forcedeath Bug
//						Speech anims interpolation
//						Moved texts a bit down
// 1.601	13/05/2001	Fixed Fireball Bug & misc bugs
// 1.600	13/05/2001	Fixed many bugs
// 1.599	12/05/2001	Fixed many bugs
// 1.598	11/05/2001	Fixed -g flag for SENDEVENT
//						Fixed ON LOAD Event refused when life==0 ... which is the case at call time...
// 1.597	08/05/2001	Added SETMATERIAL script command
// 1.596	03/05/2001	Corrected some light edition bugs
//						Added & Managed ON LOAD Event...
//						Corrected Octree Bug (Sebastien)
//						Some misc FPS upgrades
// 1.595	02/05/2001	Added LOOK_AROUND behavior (-L)
//						Implemented Fast F2L & corrected associated bug
//						Now background polys are sorted by textures before draw
//						Some misc optims in Interactive objects rendering.
// 1.594	01/05/2001	TARGET_DEATH sent on KILLME even on non-npc
// 1.591	25/04/2001	Same as 1.590
// 1.590	25/04/2001	Same as 1.589
// 1.589	25/04/2001	Same as 1.588
// 1.588	25/04/2001	Arffffffff
// 1.585	20/04/2001	Many Improvements
//						Head Cut, Torso Cut (inhibed for now)
//						Bare Hand damages
// 1.584	18/04/2001	Added BEHAVIOR script command
//						Magic Book Interface Relooked & remade
// 1.583	17/04/2001	misc debugging
// 1.582	17/04/2001	Added Crouch
//						Added Jump
// 1.582	13/04/2001	Some Last Minute Debugging........
// 1.581	13/04/2001	Some Last Minute Debugging.
// 1.580	13/04/2001	Some Last Minute Debugging.
// 1.579	13/04/2001	Minor changes for Kemal version
// 1.578	13/04/2001	Totally changed ths way the player is drawn and 1st person view
// 1.577	13/04/2001	Fixed Tweak Objects bug...
// 1.576	13/04/2001	Debugged Sound (Sort of...)
// 1.575	12/04/2001	Removed temporarily Treatzones...@@@@@ To re-add
//						Added confirm box on Danae Close...
// 1.574	12/04/2001	SET_PLAYER_TWEAK SKIN first version
//						Fixed some small bugs
//						can now strafe run
// 1.573	10/04/2001	Tweaking is on a good way to work !!
//			11/04/2001	Added new animations capabilities to player.
// 1.572	09/04/2001	Corrected Player not moving with Framerate
//						Objects are now clickable in book
//						Removed Inventory Bug (inaccessible from book)
//						Book Uses new interface
//						Objects can be equiped/unequiped
//						Equiped objects affect stats
// 1.571	29/03/2001	Added SETOBJECTTYPE script command
//						Added ISTYPE operator to IF statement
//						Added SETEQUIP script command
//						Script "player" info now refers first to IO number 0 !!!
//						Linked Objects are now using Extra Rotations too... !
//						Fixed little FTL bug...
// 1.570	21/03/2001	Final(for now) version of FTL & FTS Files
//						FTL & FTS files are saved in GAME subdirectory
//						Player now is an IO with a global script.
//						First version of new hero animation system
//						First version of global animations manager
//						Too Many news to just list here...
// 1.569	07/03/2001	Corrected Cheese Bug (Invalid ^Sender for stacked events)
//						Fixed DirectInput Acquire Bug
//						New loading system nearly OK
//						Athena soundsystem now updated by a thread
// 1.568	07/03/2001	Corrected Little SCID bug.
// 1.567	07/03/2001	arf Corrected Major GetNextWord Bug AGAIN:p
// 1.566	07/03/2001	arf Corrected Major GetNextWord Bug :p
// 1.565	07/03/2001	New CEDIT Version. (Solves 0D0A bug ???)
//						Simplified a bit GetNextWord
//						Removed Mouse Deadlock (Or So I Think...)
// 1.564	06/03/2001	Added Menu Invert Mouse
//						Added Menu Mouse Sensitivity
//						Corrected some bugs
//						Added SC partial support
//						Added SendIOEvent Stack
// 1.563	06/03/2001	Timers are now unique by name AND io
//						Direct Input implementation advanced quite a bit
//						Menu improvement (Customize keys)
// 1.562	02/03/2001	Now True Black Is Removed from JPEGs.
//						Added First Version of Direct Input (Mercury)
// 1.561	26/02/2001	Improved Physics
//						Corrected Ambiances (Athena)
//						Added Lightning (Key I & O)
// 1.559	23/02/2001	Removed variables advanced interpretation temporarily
// 1.558	23/02/2001	Runes Plays their samples in book
//						Corrected Minor Book Inconsistency (with Improvement interface)
//						1st version of lightning
//						Added TILDE '~' Notion for Forcing Script text Interpretation
//						Ambiances are now correctly reset with F9
//						Added sound to Lightning FX and is now a Spell ( O key for now )
// 1.557	20/02/2001	Removed Riched20 temporarily
//						Patched Playanim -e bug
// 1.556	20/02/2001	Cameras & Markers aren't drawn anymore
//						Changed Base player head
// 1.555	20/02/2001	Multiple Minor Upgrades
//						Added Poison ( P key )
//						Added XP/Levels Management ( X key )
//						Added Ambiances
//						Added AMBIANCE Script Function
//						Improved Physics
//						Improved Script Editor
//						Added POISON script command
// 1.554	16/02/2001	Moved Bookmarks 10 to the left
//						Corrected Nhi rune being recognised as aam in book
//						^Sender correctly filled with 0s now
//						PLAY & SPEAK now play samples the right way
//						Fixed Minor Paths inconsitencies for Sound Playing
// 1.553	15/02/2001	Corrected CheckSyntax Bug (LoopFreeze)
//						Now Bullshit at script end is automatically removed (hope so...)
//						Now Torch Follows player
//						Zoomed Book Hero
//						Fixed cursor being under things sometimes
//						Too much minor upgrades to list here...
// 1.552	09/02/2001	Fixed Teleport -p bug (moveto not updated)
//						Integrated CEditor Windows (Didier Pédréno)
//						Fixed Jump being... not as we wanted it to be !
//			10/02/2001	Implemented Nicolas' SoundSystem (Athena)
//						Many Fixes Needed to implement correctly Athena.
//			12/02/2001	Improved Book
//						Added Multifaces support for Hero
//			13/02/2001	Improved Physics Performance & consistency
//			14/02/2001	Book Improvements
//						Added ^SENDER system Variable
//						Added Runes Necklace in book
//						Added RUNE Script Command
// 1.551	08/02/2001	Fixed Priority System Locking to Idle.
//						Misc Interface Changes for Book
//						Removed Tutorial Item from menus
// 1.550	01/02/2001	Now Cursor is visible in FullScreen in Map & Menus
//			05/02/2001	Passive Thread Loading ++
//			07/02/2001	Passive Thread Loading seems OK now
//						Corrected Little Object Selection Bug
//						Corrected Sweet FireBall Bug
//						Now Player Starting stats are SuperHuman
//						Theo2Eerie is now quite faster
// 1.549	29/01/2001	Continuing Book Upgrades
//			31/01/2001	Temporarily Removed Animation Sounds in an attempt to locate Raf's Bug
//			01/02/2001	Raf's Bug seems corrected (ReleaseAnim bug)
//						Put back Animation Sounds
// 1.548	26/01/2001	Book Is now Clickable for character upgrades
// 1.547	26/01/2001	Corrected MAJOR Loop Bug in ChekScriptSyntax (goto bug)
// 1.546	23/01/2001	Improved the book (now depends on resolution for everything)
//						Started to code The New Quest menu option
//			24/01/2001	Continued to code New Quest menu option
//						Removed MAP bug (interdraw) ... or at least I think so...
//						Fixed INVENTORY2_OPEN buggie
//			25/01/2001	Secured "Reload All Scripts" Options proving unstable while game was running
//						Coded Pathfinder Request Queue. Multi-pathfinding now works.
//						Added "Fast Loads" Option
//						Fixed Marco's Bug (Level 0 PutInFrontOfPlayer)
// 1.545	22/01/2001	Added Basic Menus Handling
//						Can now disable INVENTORY2_OPEN with SETEVENT
// 1.544	19/01/2001	Small Physics Corrections
//						Optimized Moving-Cylinder Collision Algorithm speed
//						Fixed little SENDEVENT bug (not sending Pre-defined Messages)
//						Fixed PLAYANIM -e bug
// 1.543	17/01/2001	Corrected Misc ScreenSaver Bug (Didier Pedreno)
//						Coded Objects Physics Basis
//						Improved Object Physics Collisions (need optims)
//						Started to code Objects-Objects Collisions
//			18/01/2001	Added INITEND Script Event which occurs at IO Initialisation End
//						Added TWEAK ICON script command
//						Fixed ^Dist bugs when objects were in inventories
//						Fixed Major GetSystemVar bug on Float walues
//						Added -Z & -R flags to SENDEVENT script command (Zone & Radius)
//						Corrected SetEditMode Bug (preventing some commands from executing ON INIT)
//			19/01/2001	Removed EE_RTP3 (useless) & Moved _YXZRotateNorm & _YXZRotatePoint in EERIEMath.h
//						Increased Dynamic_Normals Performance by removing unneeded computations
//						Added MARKERS Handling
//						Added TELEPORT Script Command
// 1.542	04/01/2001	Corrected Misc Physics Bugs
//						More Realistic Jump
//						FireBall generates a force on player
// 1.541	03/01/2001	Player Physics are improving
//						Better Mouse Handling (MouseGrab)
//			04/01/2001	Major Project Reorganisation
//						Better Surface Sliding
// 1.540	29/12/2000	Physics are Under big changes
//						Player Collision/Movements uses Cylinder/velocities/forces
//						Added Jump Capabilities (Beta Version).
// 1.539	27/12/2000	Corrected Treatzone not handled for Hidden IO
//						Some serious Project Reorganisation
//						Added VISTA VITAE Spell (Detect_Living)
// 1.538	26/12/2000	Corrected Local/Global strings interpretation bug
//						Herosay can now say String Variables Content
// 1.537	26/12/2000	Modified Animation System Correction II
// 1.536	26/12/2000	Modified Animation System for 1st Person View (loop bug)
//						Changed default Player WAIT animation
// 1.535	22/12/2000	Added JPEG and PNG Support for texture loading
//						Removed Major Texture Restore Bug (8bpp)
// 1.534	22/12/2000	Fixed Speech Flicker
//						Cooler Sausage CameraControl
//						Corrected Combat/Magic Mode Bug (Looped  Anims)
//						More Precise Light Positionning (but slower...)
// 1.533	21/12/2000	Temporarily Changed BKG Metal FX
//						MapMode Switching Particles Restore
//						Changed Cursors Look/focus
//						Added Anims TALK_NEUTRAL TALK_HAPPY & TALK_ANGRY to LOADANIM
//						SPEAK -H & -A (Happy/Angry)
//						Changed Launchdemo Button
//						Changed MIPMap Levels handling (LODBias)
//						Changed ANCHORS creation and Linking (Diagonals)
//						Added CAMERACONTROL Script Command
//						Inventory & Book now follow PLAYER_INTERFACE command directives
// 1.532	20/12/2000	Improved SCRIPT Math Handling (MUL/DIV/INC/DEC)
//						Added PLAYERLOOKAT io Script Command
//						Added PLAYANIM [-x] NONE to stop animation
//						Added PATHFINDER_SUCCESS PATHFINDER_FAILURE PATHFINDER_END Events
//						Changed COMBINE Action Appearance (Ghost of object under cursor)
//						Can Now SKIP Conversation by pressing SPACE
// 1.531	19/12/2000	Coded CONVERSATION system
//						Now IO & Player follows the ground more acurately
//						Changed ManageNPCMovement/CheckNPC/misc... position in code
//						Corrected LastGroup Bug
// 1.530	11/12/2000	Started to implement Multiple Layer Animation.
//			12/12/2000	Finished first implementation of Multiple Layer Animation (BETA status).
//						Huge Re-Ordering of Project
//			13/12/2000	Corrected ADDFROMSCENE bug (not visible in editor mode)
//			18/12/2000	Added PLAYERADD PLAYERADDFROMSCENE PLAYERADDMULTI to INVENTORY Command
//						Corrected Bread Patching Bug
//						Corrected LinkObjToObj Bug (code enforcement)
//						Corrected Cinemascope/Player_Interface Bug (Overriden by Player Controls)
//						Corrected Magic Draw Memorisation Bug
//						Added Random string Handling
//						Changed Default Font to a more lisible one (comic sans Ms)
// 1.529	07/12/2000	Corrected MAJOR Object Link Bug
// 1.528	07/12/2000	Corrected Minor bugs in EERIETexture
//						Corrected OBJECT_HIDE bug (not showing object back in editor mode)
//						Corrected MAJOR SPEECH bug
// 1.527	06/12/2000	Corrected Light Color bug (spawning multi-dynlights)
//						Corrected BIG patch bug (locname patch 255)
// 1.526	06/12/2000	Changed default FIRE/FIRE2 configs
// 1.525	05/12/2000	Upgraded/corrected DynLights Dialog
//						Added ^ARXDAYS ^ARXHOURS ^ARXMINUTES ^ARXSECONDS
//						Added ^GAMEDAYS ^GAMEHOURS ^GAMEMINUTES ^GAMESECONDS
//						Added ^ARXTIME_HOURS ^ARXTIME_MINUTES ^ARXTIME_SECONDS
//			06/12/2000	Fixed Bugs in Lights
//						Now last project path is saved in registry and restored at startup
//						Memorizes some windows Pos in registry.
// 1.524	04/12/2000	Now cannot click on Hidden Objects.
//						Added WORLD_FADE command
// 1.523	04/12/2000	Corrected REPLACEME little bug (Initpos not initialized correctly).
//						Added OBJECT_HIDE script command
//						Localisation strings are now loaded in memory
//						Corrected Localisation string bug (overflow)
//						Added ISIN operator to IF command (!= ISCLASS)
// 1.522	30/11/2000	Added Zone Path.handling
//			01/12/2000	Improved Misc script commands & Zone commands/management
//						Added QUAKE script command
// 1.521	29/11/2000	Upgraded Clothes Handling
//						Now IO Illumination Normals are cooler :) (removed aproximative Method)
// 1.52		27/11/2000	Added Unfreeze All IO Menu
//						Added Mesh Simplification Window & Menu
//			28/11/2000	Improved Mesh Simplification Algorithm
//						Changed Tweak Skin Syntax & Behaviour
//						Corrected Minor Bugs in Mesh Tweaking
//						Corrected Misc Bugs here & there
//						//Uses Collision Spheres for IO fake shadow-casting
//						//Uses ASM FLOAT2LONG on some funcs...
// 1.519	24/11/2000	Added Script Syntax Checker
//						Corrected Script command '--' increasing value instead of decreasing it
// 1.518	23/11/2000	Added Wrap-Mode Handling for background textures
//						Upgraded IO Normals calcultions by taking into account polys surface size
//						Accelerated Slightly IO Load by avoiding to send too much "ON_INIT" events
//						Corrected INVENTORY ADD_FROM_SCENE potential Bug if no inventory was created
// 1.517	21/11/2000	Re-Corrected 1.516 Bug
// 1.516	21/11/2000	Corrected Inter Reinit Bug (Keeping Invalid curanim pointer after F9)
// 1.515	17/11/2000	Corrected Over-Bug Cinematic. Added Nhi Tempus Spell
// 1.514	17/11/2000	Fixed IO Multi-Draw  (Reason was: ManageNPCMovement(inter.iobj[i]);)
// 1.513	17/11/2000	Improved Drain Life Spell effect
//						Fireball Impact force on player
// 1.512	17/11/2000	Added Magic Dust + Hide/Show
// 1.511	17/11/2000	Upgraded Clipping for cinematics look up/down
//						Added 'Cancel Snapshot' to Snapshot Options


//////////////////////////////////////////////////////////////////////////////
// SEBASTIEN Versions History												//
//////////////////////////////////////////////////////////////////////////////
// 03/12/2001	Hummmm Blah Blah, en fait j'ai oublie de le mettre à jour, voilà! :)
//              et pis comme y a sourcesafe ça ne sert plus à rien, et comme je suis
//				feignant bah voilà!! :)
// 18/01/2001	Blah Blah


//////////////////////////////////////////////////////////////////////////////
// DIDIER Versions History													//
//////////////////////////////////////////////////////////////////////////////
//
// 09/01/02		autorun -> setup_langue.exe
//
// 08/01/02		des trucs sur l'interface
//				des trucs sur les clicks souris
//				des trucs sur les textures
//				des trucs sur le firstrun, et l'autorun
// 07/01/02		les render states
//				16 bits 565 pour les textures contre 555 avant
//				interface cleanage
//				textures, state color key
//				init choix de la langue
//				interface mouse et stuff
// 04/01/02		les clicks
//				bouton droit sur les shops
//				toogle bouton gauche -> description
//				bug description sur un laché de drag
// 03/01/02		les clicks
//				chinese windows
//				interface bugs
//				...
// 02/01/02		fix autorun lance le bon readme en fonction de la langue
//				correspondance langue - contenu du menu
//				config touches -> fix conversion unicode
//				tourner le perso dans la fiche de création de perso
//				inventory with stealth -> multi pick
//				secondary -> ferme big player inventory et inversement
//				fix textures voodoo 3 (max size 256)
//				intuition pour les prix + à la vente, - à l'achat
//				mouselook + drag -> use drag item qd on lache le mouselook -> fixé
//				fix leak sur les spells
// 20 & 21 dec 2001		wise
//						fix autorun multi langues
// 18 & 19 dec 2001		interface popup choix des langues
// 13->17/12/2001	steal npc
//				bugs z interface
//				text affiché partout
//				menu
//				low resize texture
//				direct input qui manquait à droite à gauche
//				esc skip intro et tout
//				couille d'anim sur les spell scrolls
// 12/12/2001	fix interface z order
//				cross not over dragged object if over inventory icon
//				multiinventaire drag sur fleche ouvre le suivant
//				touches config magic mode
//				stealth icon z order
//				menu scroll timé
//				no resize zmaps refinement
//				precast fixé, ct pas lancé sur le player, ct lancé sur le scroll ... huh
//				mouse toogle note bouton droit fixé
// 11/12/2001	gold drop partout -> bourse player
//				fix gold dans secondary inventory -> un seul élément cumulé
// 10/12/2001	skip avi fishtank
// 07/12/2001	steal avec script
// 06/12/2001	ben moi je mets à jour des fois :o) pas comme seb, ni chabi la bibar
//				mega inventaire stuff (click sound + drag fleches + open book)
//				esc menu -> direct input
//				divers affichage de texte partout (break d3d)
//				bug secondary inventory x>=0 (1ere colonne)
//				rezize texture -> à refaire le stretch (dd->blt)
// 05/12/2001	inventaire toutes les features
// 04/12/2001	triple inventaire
//				fix interface
// 03/12/2001	small fixes
//				hash table ++
// 30/11/2001	spellcast
//				book fx fix valeur hardcodée
// 29/11/2001	bug items derriere le player book/notes
//				bug level up bloque souris
//				delete quest -> no save game (avt new save game)
//				final release dans le menu (back creation perso)
//				mesh reduction hidé
//				book fx new spell multi res
//				can cast spell flags FSMD et !player
//				loadquest en additif fixé
// 28/11/2001	fix bug localisation
//				bug spells cant cast
//				fix notes/book
//				fix mouse stuff
//				menu stuff (resume -> text)
//
// 27/11/2001	bug mouseinrect bourse drop gold
//				drag = action / chat
//				mouse states avec seb
//				gros cosmetic interface
//				misc bugs time, mouse
//				sevy day
// 26/11/2001	infos fonts dans utext_*.ini
//				anti rebond des touches action item (potion, torche)
//				bourse halo
//				spells to memorize + halo
//				aim_hit
//
// 22/11/2001	stricmp pour le hash de la localisation
//				torch shortkey
//				identify des objets + equipement du player
// comme quoi je mets à jour des fois :o)
// 21/11/2001   Halos + book explosion de partoches qd new spell
// 20/11/2001	Unicode en hash from memory
// 19/11/2001	Unicode
// fix des lights -> précision théo à 5 près (meme vertex)
// intégration des lights
// books
// interface
// fonts
// passage unicode pour le milestone localisation
// fonts
// menu2
// interface
// découpage de sons (...)
// script debugger
// divers spells
// particules, particle manager, & system
// script editor
// 23/01/2001	Light Model version 1 (enhanced vertex light ray cast)
// 18/01/2001	Blah Blah


//////////////////////////////////////////////////////////////////////////////
// NICOLAS Versions History													//
//////////////////////////////////////////////////////////////////////////////
// 03/12/2001	Oups, ben pareil que seb a ceci pres que je viens de le decouvrir :)
// 18/01/2001	Blah Blah

