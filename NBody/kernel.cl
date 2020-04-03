struct body{
	float4 pos_vel;
	float mass;
};

__kernel void move(__global struct body* bodies, float delta_time)
{
	uint id = get_global_id(0);

	float2 acceleration = (float2)(0.0, 0.0);
	for(uint i = 0; i < get_global_size(0); i++){
		if(i == id) { continue; }

		float2 dist = bodies[i].pos_vel.xy - bodies[id].pos_vel.xy;
		float dist_len = length(dist);	
		acceleration += bodies[i].mass * dist / (dist_len * dist_len * dist_len);//normalize(dist) / (dist_len ^ 2)
	}

	bodies[id].pos_vel.zw += acceleration * delta_time;
	bodies[id].pos_vel.xy += bodies[id].pos_vel.zw * delta_time;
}