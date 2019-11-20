function airconditioner_semantic_to_evergrande(str, method, req_id, user_id, dev_uuid)
    local cjson = require "Lua.dkjson"
    local table_org = cjson.decode(str)
    local category = "airconditioner"
    local table_evergrande = {}
    local table_params = {}
    local table_evergrande_attr = {}
    local node_id = nil
    local table_org_attr = table_org["attr"]
    local table_evergrande_params = {}

    table_evergrande["method"] = method
    table_evergrande["req_id"] = req_id
    for k, v in pairs(table_org_attr) do
        if string.find(k, "switch") then
            node_id = k;
            if (table_org["attr"]["switch"] == "打开") then
                table_evergrande_attr["switch"] = "on"
            elseif (table_org["attr"]["switch"] == "关闭") then
                table_evergrande_attr["switch"] = "off"
            end
        elseif string.find(k, "temperature") then
            node_id = k;
            table_evergrande_attr["temperature"] = table_org_attr["temperature"]
        end
    end
    table_evergrande["nodeid"] = category..".main."..node_id
    print(table_evergrande["nodeid"])
    table_evergrande_params["user_id"] = user_id
    table_evergrande_params["device_uuid"] = dev_uuid
    table_evergrande_params["attribute"] = table_evergrande_attr;
    table_evergrande["params"] = table_evergrande_params
    --[[
    table_evergrande["params"]["user_id"] = user_id
    table_evergrande["params"]["device_uuid"] = dev_uuid
    table_evergrande["params"]["attribute"] = table_evergrande_attr;
    --]]
    local evergrande_cjson = require("Lua.dkjson")
    local evergrand_json_str = evergrande_cjson.encode(table_evergrande)
    print(evergrand_json_str)
    return evergrand_json_str
end

--local str = "{\"category\":\"airconditioner\",\"attr\":{\"room\":\"客厅\",\"switch\":\"打开\"}}"
--local str = "{\"category\":\"airconditioner\",\"attr\":{\"room\":\"客厅\",\"temperature\":25}}"
--local method = "dm_set"
--local req_id = 178237278
--local user_id = 2003
--local dev_uuid = "112233445566778810"
--airconditioner_semantic_to_evergrande(str, method, req_id, user_id, dev_uuid)