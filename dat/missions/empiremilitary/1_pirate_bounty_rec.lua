--[[

   Pirate Bounty

   Randomly appearing bar mission to kill a unique pirate. Heavily based on bobbens's Pirate Bounty mission for Naev

   Authors: Kinniken, bobbens

--]]

include "dat/missions/templates/ship_kill.lua"
include "dat/missions/supportfiles/pirates.lua"
include('universe/objects/class_planets.lua')
include "universe/live/live_universe.lua"

   -- Whether mission starts in bar (if false, it starts in computer)
   mission_bar=false
   -- Whether mission ends in bar (if not, it ends in space in start system)
   mission_return_to_bar=false

   computer_title  = "NAVY: Pirate Activity"

   -- Mission details
   misn_title  = "Eliminate ${targetShipName}"
   misn_desc   = [[ 
   Pirate ship ${targetShipName} needs 'investigation'.
   Last seen in system ${targetSystem}.
   Common friend suggested your line of work.
   Contribution available to cover maintenance costs.]]

   space_success_title="One More Notch"
   space_success_text="As you enter ${endSystem}, you beam proof of your 'investigation' back to ${endPlanet} on a secure line. Minutes later a coded acknowledgement is sent back along with your fee of ${credits} credits. Job done!"

-- Scripts we need
include("pilot/pirate.lua")


function create ()
   -- Note: this mission does not make any system claims.
   -- Create the target pirate
   target_ship_name, target_ship, target_ship_outfits,target_ship_ai,target_ship_faction = pir_generate()

   -- Get target system
   target_system = get_suitable_system( system.cur() )

   -- Get credits
   credits  = rnd.rnd(10,30) * 10000

   template_create()
end


--[[
Mission entry point.
--]]
function accept ()
   template_accept()
end

-- Gets a new suitable system
function get_suitable_system( sys )
   local newsys=get_nearby_pirate_system(sys)
   if newsys then
      return newsys
   else
      return sys
   end
end

-- Player won, gives rewards.
function give_rewards ()

   -- Give factions
   faction.modPlayerSingle( G.EMPIRE, 1 )
   faction.modPlayerSingle( G.PIRATES, -5 )
   
   local planet=planet_class.load(start_planet)

   if (planet.lua.settlements.humans) then
      planet.lua.settlements.humans.services=planet.lua.settlements.humans.services*1.2
   end
   if (planet.lua.settlements.natives) then
      planet.lua.settlements.natives.services=planet.lua.settlements.natives.services*1.2
   end
   planet:addHistory("The killing of notorious pirate "..target_ship_name.." has boosted local trade.")
   generatePlanetServices(planet)
   planet:save()
   
   template_give_rewards()
end