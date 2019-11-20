#ifndef INCLUDE_GUARD_LS_SVG
#define INCLUDE_GUARD_LS_SVG

enum svg_element_ {
    SvgElement_Null,
    SvgElement_Rect,
    SvgElement_Circle,
    SvgElement_Ellipse,
    SvgElement_Line,
    SvgElement_Count,
};

struct svg_v2 {
    r32 x,y;
};

struct svg_path_segement {
    svg_v2 P1, P2, C1, C2;
};

struct svg_circle {
    svg_v2 Center;
    r32 R;
};

struct svg_rect {
    svg_v2 P;
    svg_v2 Dim;
};

struct svg_path {
    svg_path_segement *Segments;
    u32 SegmentCount;
};

struct svg_element {
    svg_element_ Type;

    union {
        svg_path Path;
        svg_rect Rect;
        svg_circle Circle;
    };
};

struct svg {
    svg_element *Elements;
    u32 ElementCount;
};

enum svg_parsing_mode_ {
    SvgParsingMode_Tag,
    SvgParsingMode_Props,
};

svg
SvgParse(u8 *Data, u32 Size)
{
    svg Svg = {};

    svg_parsing_mode_ Mode = SvgParsingMode_Tag;
    ls_parser String((char *)Data, Size);
    ls_string TagName;

    token Token;

    while (String.RemainingBytes()) {

        if (Mode == SvgParsingMode_Tag) {
            Token = String.GetToken();

            // expecting tag
            assert(Token.Type == Token_LessThan);

            token Next = String.PeekToken();

            b32 ClosingTag = false;

            if (Next.Type == Token_ForwardSlash) {
                String.GetToken();
                ClosingTag = true;
            }

            if (!ClosingTag) {
                Mode = SvgParsingMode_Tag;

                Token = String.GetToken();
                TagName = Token.Text;

                if (TagName == "svg") {
                    //
                } else if (TagName == "path") {
                    //
                }

                printf("<%.*s>\n", TagName.Size, TagName.Data);

                Mode = SvgParsingMode_Props;
            } else {
                String.RequireToken(Token_Identifier);
                String.RequireToken(Token_GreaterThan);
            }
        } else if (Mode == SvgParsingMode_Props) {
            Token = String.GetToken();

            if (Token.Type == Token_Identifier) {
                printf("    %.*s\n", Token.Text.Size, Token.Text.Data);

                String.RequireToken(Token_Equals);

                Token = String.GetToken();
                assert(Token.Type == Token_String);

                // ParseProperty(TagName, Token.Text);
            } else {
                if (Token.Type == Token_ForwardSlash) {
                    // non-paired tag
                    Token = String.GetToken();
                }

                assert(Token.Type == Token_GreaterThan);
                Mode = SvgParsingMode_Tag;
            }
        }
    }

    return Svg;
}

#endif // INCLUDE_GUARD_LS_SVG