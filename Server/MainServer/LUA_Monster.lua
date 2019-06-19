myid = 99999;
mytype = -1;
	
function set_uid(x)
		myid = x;
end

function set_type(x)
		mytype = x;
end

function Event_OnlyCallCPP(player, Monster, type)
	API_Process(player, Monster, type)
end

function Event_ChaseOrAttack(player, Monster, type)
	isLive = API_get_IsLive(player);
	if(isLive) then
		player_x = API_get_x(player);
		player_y = API_get_y(player);
		my_x = API_get_x(Monster);
		my_y = API_get_y(Monster);

		if(type == 1) then
			if ((player_x - my_x) < 2 and (player_x - my_x) > -2) then
				if ((player_y - my_y) < 2 and (player_y - my_y) > -2) then
					API_do_attack(player, myid);
					return 1;
				end
			end
			API_do_chase(player, myid);
		elseif(type == 2) then
			if ((player_x - my_x) < 4 and (player_x - my_x) > -4) then
				if ((player_y - my_y) < 4 and (player_y - my_y) > -4) then
					API_do_attack(player, myid);
					return 1;
				end
			end
			API_do_chase(player, myid);
		elseif(type == 3) then
			if ((player_x - my_x) < 6 and (player_x - my_x) > -6) then
				if ((player_y - my_y) < 6 and (player_y - my_y) > -6) then
					API_do_attack(player, myid);
					return 1;
				end
			end
			API_do_chase(player, myid);
		end
	else
		API_go_SpawnPosition(player, myid);
	end
	return 2;
end
