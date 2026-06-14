forcePower = 20.0

function onUpdate(entityId, dt)
    local rb = Scene:getRigidbody(entityId)
    
    -- Находим сущность камеры (можно по имени или тегу)
    local camEntityId = Scene:findEntityByName("Empty")
    local camComp = Scene:getCameraComponent(camEntityId)
    local camera = camComp:getCamera()
    
    -- Куда смотрит камера?
    local camDir = camera:getDirection()
    
    -- Векторы движения
    local forward = Vector3.new(camDir.x, 0, camDir.z)
    forward:normalize()
    local right = Vector3.new(forward.z, 0, -forward.x)
    
    local moveDir = Vector3.new(0, 0, 0)
    if Keyboard:isKeyHeld("W") then moveDir = moveDir + forward end
    if Keyboard:isKeyHeld("S") then moveDir = moveDir + (forward * -1) end
    if Keyboard:isKeyHeld("A") then moveDir = moveDir + right end
    if Keyboard:isKeyHeld("D") then moveDir = moveDir + (right * -1) end

    if moveDir:length() > 0.01 then
        moveDir:normalize()
        rb:addForce(moveDir * forcePower)
    end
end