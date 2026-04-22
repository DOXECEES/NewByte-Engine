-- скорость движения (единицы/сек)
local speed = 5.0

function onUpdate(entityId, dt)

    local transform = Scene:getTransform(entityId)
	local rb = Scene:getRigidbody(entityId)

	if Keyboard:isKeyHeld("W") then
		rb:addImpulse(Vector3.new(0, 0, 10)) 
	end

	if Keyboard:isKeyHeld("D") then
		rb:addForce(Vector3.new(10, 0, 0)) 
	end
	
	if Keyboard:isKeyHeld("A") then
		rb:addForce(Vector3.new(-10, 0, 0)) 
	end

	if Keyboard:isKeyHeld("S") then
		rb:addForce(Vector3.new(0, 0, -10)) 
	end

	local pos = transform.position

end