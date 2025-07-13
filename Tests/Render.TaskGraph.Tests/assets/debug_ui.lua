-- color
BLACK={ 0.1, 0.1, 0.1, 1.0 }
BLUE={ 0.4, 0.5, 0.6, 1.0 }
GRAY={ 0.2, 0.2, 0.2, 1.0 }
GRAY2={ 0.32, 0.32, 0.32, 1.0 }
RED={ 0.8, 0.2, 0.2, 1.0 }
---------------
--- font
default_font="Fonts/Noto-Sans-S-Chinese/NotoSansHans-Regular-2.ttf"
--------------





local function value_or(value, default)
    if value == nil then
        return default
    else
        return value
    end
end
local function text(args)
    local id=args.id
    local width = args.width
    local height = args.height
    local text= args.text
    local font_size = value_or(args.font_size, 24)
    local font_color = value_or(args.font_color, { 1.0, 1.0, 1.0 })
    local font_path= value_or(args.font, default_font)
    return {
        id = id,
        type = "text",
        width = width,
        height = height,
        size = font_size,
        text = text,
        font_color = font_color,
        font_path=font_path,
    }

end
local function button(args)
    local width = args.width
    local height = args.height
    local tag = args.tag
    local color = args.color
    local id = args.id
    local font_size=value_or(args.font_size,24)
    local    font_color = value_or(args.font_color, { 1.0, 1.0, 1.0 })

    return {
        id = id,
        type = "quad",
        width = width,
        height = height,
        color = color,
        content = {
            text{
                type = "text",
                width = "100%",
                height = "100%",
                font_size = font_size,
                text = tag,
                font_color=font_color,

            }
        }
    }
end





return {
    visible=true,
    type = "grid",
    width = "100%",
    height = "100%",
    id="root",
    content = {
        button { width = 270, height = 32, tag = "Test TaskGraph Compile", color = GRAY, id = "btn:Test TaskGraph Compile" ,font_size=24,font_color=WHITE},



    },
}
