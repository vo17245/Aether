
function init()
    storage().cnt=0;
end
function is_first_run()
    if storage().is_first_run==nil then
        storage().is_first_run=true
        return true
    end
    return false
end
if is_first_run() then
    init()
end
storage().cnt=storage().cnt+1
log_info("cnt: "..tostring(storage().cnt))
log_info("camera position:"..tostring(camera.position[1]).." "..tostring(camera.position[2])
.." "..tostring(camera.position[3]))

set_camera_position(camera.position[1]-0.01,camera.position[2],camera.position[3])