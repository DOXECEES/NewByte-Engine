
function onTriggerEnter(entityId)

	local rb = Scene:getRigidbody(entityId)
	
	rb:addForce(Vector3.new(0, 100, 0)) 

end