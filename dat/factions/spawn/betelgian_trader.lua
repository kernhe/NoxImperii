include("dat/factions/spawn/common.lua")


-- @brief Spawns a small trade fleet.
function spawn_patrol ()
    local pilots = {}
    local r = rnd.rnd()

    if r < 0.5 then
       scom.addPilot( pilots, "Betelgian Traders Pelegrina", 10 );
    elseif r < 0.8 then
       scom.addPilot( pilots, "Betelgian Traders Pelegrina", 10 );
       scom.addPilot( pilots, "Betelgian Traders Carvela", 20 );
    else
       scom.addPilot( pilots, "Betelgian Traders Carvela", 20 );
       scom.addPilot( pilots, "Betelgian Traders Veniera", 50 );
    end

    return pilots
end


-- @brief Spawns a larger trade fleet.
function spawn_squad ()
    local pilots = {}
    local r = rnd.rnd()

    if r < 0.5 then
       scom.addPilot( pilots, "Betelgian Traders Pelegrina", 10 );
       scom.addPilot( pilots, "Betelgian Traders Carvela", 20 );
       scom.addPilot( pilots, "Betelgian Traders Veniera", 50 );
    elseif r < 0.8 then
       scom.addPilot( pilots, "Betelgian Traders Pelegrina", 10 );
       scom.addPilot( pilots, "Betelgian Traders Carvela", 20 );
       scom.addPilot( pilots, "Betelgian Traders Carvela", 20 );
       scom.addPilot( pilots, "Betelgian Traders Veniera", 50 );
    else
       scom.addPilot( pilots, "Betelgian Traders Pelegrina", 10 );
       scom.addPilot( pilots, "Betelgian Traders Carvela", 20 );
       scom.addPilot( pilots, "Betelgian Traders Carvela", 20 );
       scom.addPilot( pilots, "Betelgian Traders Veniera", 50 );
       scom.addPilot( pilots, "Betelgian Traders Impresa", 150 );
    end

    return pilots
end


-- @brief Creation hook.
function create ( max )
    local weights = {}

    -- Create weights for spawn table
    weights[ spawn_patrol  ] = 100
    weights[ spawn_squad   ] = math.max(1, -80 + 0.80 * max) 
   
    -- Create spawn table base on weights
    spawn_table = scom.createSpawnTable( weights )

    -- Calculate spawn data
    spawn_data = scom.choose( spawn_table )

    return scom.calcNextSpawn( 0, scom.presence(spawn_data), max )
end


-- @brief Spawning hook
function spawn ( presence, max )
    local pilots

    -- Over limit
    if presence > max then
       return 5
    end
  
    -- Actually spawn the pilots
    pilots = scom.spawn( spawn_data )

    -- Calculate spawn data
    spawn_data = scom.choose( spawn_table )

    return scom.calcNextSpawn( presence, scom.presence(spawn_data), max ), pilots
end
