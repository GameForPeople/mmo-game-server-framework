myid = 99999;
mytype = -1;
	
function set_uid(x)
		myid = x;
end

function set_type(x)
		mytype = x;
end

function Event_ChaseOrAttack(player)
	isLive = API_get_IsLive(player);
	if(isLive) then
		player_x = API_get_x(player);
		player_y = API_get_y(player);
		my_x = API_get_x(myid);
		my_y = API_get_y(myid);

		if(mytype == 1) then
			if ((player_x - my_x) < 2 && (player_x - my_x) > -2) then
				if ((player_y - my_y) < 2 && (player_y - my_y) > -2) then
					API_do_attack(player, myid);
					return;
				end
			end
			API_do_chase(player, myid);
		elseif(mytype == 2) then
			if ((player_x - my_x) < 4 && (player_x - my_x) > -4) then
				if ((player_y - my_y) < 4 && (player_y - my_y) > -4) then
					API_do_attack(player, myid);
					return;
				end
			end
			API_do_chase(player, myid);
		elseif(mytype == 3) then
			if ((player_x - my_x) < 6 && (player_x - my_x) > -6) then
				if ((player_y - my_y) < 6 && (player_y - my_y) > -6) then
					API_do_attack(player, myid);
					return;
				end
			end
			API_do_chase(player, myid);
		end
	else
		API_go_SpawnPosition(player, myid);
	end
	return;
end