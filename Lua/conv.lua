--[[
function jsonstrparse(str)
    --str="{\"category\": \"EVERGRANDE.airconditioner\", \"intentType\": \"custom\"}"
    local cjson = require ("Lua.dkjson")
    local json = cjson.decode(str)
    local category = json["category"]
    local nodeid = nil
    local intent = nil
    if (category == "EVERGRANDE.airconditioner") then
        nodeid = "airconditioner"..".main.switch"
        print(nodeid)
    end
	intent = json["semantic"][1]["intent"]
	local value=json["semantic"][1]["slots"][2]["value"];
	print("name:"..value)
    --local hd_cjson = cjson.new()
	local hd_cjson = require("Lua.dkjson")
    local lua_obj = {
        ["category"] = category,
        ["intent"] = intent,
        ["value"] = value,
    }
    --print(hd_cjson.encode(lua_obj))
	return hd_cjson.encode(lua_obj)
end
--local samplejson = {"category": "EVERGRANDE.TV", "intentType": "custom"}
--jsonstrparse(samplejson)
local str = jsonstrparse()
print("str:"..str)
--]]
function airconditioner_semantic(str)
    local cjson = require "Lua.dkjson"
    local json = cjson.decode(str)
    local category = string.gsub(json["category"], "EVERGRANDE[.](%a+)", "%1")
    local table_slots = json["semantic"][1]["slots"]
    local table_attr = {}
    local table_final = {}
    print("num:"..#table_slots);
    for k,v in pairs(table_slots) do
        print(table_slots[k]["name"])
        if (table_slots[k]["name"] == "switch") then
            table_attr["switch"] = table_slots[k]["normValue"]
            print(table_attr["switch"])
        elseif (table_slots[k]["name"] == "room") then
            table_attr["room"] = table_slots[k]["normValue"]
        elseif (table_slots[k]["name"] == "temperature") then
             table_attr["temperature"] = table_slots[k]["normValue"]
        end
    end

    table_final["category"] = category
    table_final["attr"] = table_attr
    local hd_cjson = require("Lua.dkjson")
    return hd_cjson.encode(table_final)
end

function semantic_parse(str)
--[[
    local str="{\
        \"category\": \"EVERGRANDE.airconditioner\",\
        \"intentType\": \"custom\",\
        \"rc\": 0,\
        \"semanticType\": 1,\
        \"service\": \"EVERGRANDE.airconditioner\",\
        \"uuid\": \"atn06a51f34@dx000610e38356a11101\",\
        \"semantic\": [\
            {\
                \"entrypoint\": \"ent\",\
                \"intent\": \"setOnOff\",\
                \"score\": 0.8631743788719177,\
                \"slots\": [\
                    {\
                        \"begin\": 0,\
                        \"end\": 2,\
                        \"name\": \"categoryName\",\
                        \"normValue\": \"空调\",\
                        \"value\": \"空调\"\
                    },\
                    {\
                        \"begin\": 2,\
                        \"end\": 3,\
                        \"name\": \"switch\",\
                        \"normValue\": \"打开\",\
                        \"value\": \"开\"\
                    },\
                    {\
                        \"begin\": 6,\
                        \"end\": 9,\
                        \"name\": \"room\",\
                        \"normValue\": \"客厅\",\
                        \"value\": \"客厅\"\
                    }\
                ],\
                \"template\": \"的{categoryName}{switch}\"\
            }\
        ]\
        }";
--]]
    local cjson = require "Lua.dkjson"
    local json = cjson.decode(str)
    local category = string.gsub(json["category"], "EVERGRANDE[.](%a+)", "%1")
    if (category == "airconditioner") then
        return airconditioner_semantic(str)
    end
end
--local samplejson = {"category": "EVERGRANDE.TV", "intentType": "custom"}
--jsonstrparse(samplejson)
local str = jsonstrparse()
print("str:"..str)