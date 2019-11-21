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

enum svg_segment_ {
    SvgSegment_Line,
    SvgSegment_QuadraticBezier,
    SvgSegment_CubicBezier,
    SvgSegment_Elliptic,
};

struct svg_path_segement {
    svg_segment_ Type;

    union {
        struct {
            svg_v2 P1, P2, C1, C2;
        };
        struct {
            svg_v2 R;
            r32 Angle;
            u32 LargeArc;
            svg_v2 NextP;
        };
    };
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
    b32 Closed;
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

enum svg_path_command_ {
    SvgPathCommand_Null,

    SvgPathCommand_Move, // 'M'
    SvgPathCommand_LineTo, // 'L'
    SvgPathCommand_HorizontalLine, // 'H'
    SvgPathCommand_VerticalLine, // 'V'
    SvgPathCommand_CubicBezier, // 'C'
    SvgPathCommand_SmoothCubicBezier, // 'S'
    SvgPathCommand_QuadraticBezier, // 'Q'
    SvgPathCommand_SmoothQuadraticBezier, // 'T'
    SvgPathCommand_EllipticalArc, // 'A'

    SvgPathCommand_MoveRel,  // 'm'
    SvgPathCommand_LineToRel,  // 'l'
    SvgPathCommand_HorizontalLineRel,  // 'h'
    SvgPathCommand_VerticalLineRel,  // 'v'
    SvgPathCommand_CubicBezierRel,  // 'c'
    SvgPathCommand_SmoothCubicBezierRel,  // 's'
    SvgPathCommand_QuadraticBezierRel,  // 'q'
    SvgPathCommand_SmoothQuadraticBezierRel,  // 't'
    SvgPathCommand_EllipticalArcRel,  // 'a'

    SvgPathCommand_ClosePath, // 'Z or z'

    SvgPathCommand_Count,
};

svg_path_command_ CharCommandMap[128] = {};

svg_v2
SvgParseV2(ls_parser P)
{
    token Token = P.GetToken();
    assert(Token.Type == Token.Real);
    r32 x = Token.Real;

    Token = P.GetToken();
    assert(Token.Type == Token.Real);
    r32 y = Token.Real;

    return {x,y};
}

r32
SvgParseFloat(ls_parser P)
{
    token Token = P.GetToken();
    assert(Token.Type == Token.Real);
    r32 Value = Token.Real;
    return Value;
}

void
SvgAddLineSegment(svg_v2 StartP, svg_v2 EndP)
{
    svg_path_segement S;
    S.Type = SvgSegment_Line;
    S.P1 = StartP;
    S.P2 = EndP;
}

void
SvgAddCubicBezierSegment(svg_v2 StartP, svg_v2 EndP, svg_v2 Control1, svg_v2 Control2)
{
    svg_path_segement S;
    S.Type = SvgSegment_CubicBezier;
    // S.P1 = StartP;
    // S.P2 = EndP;
}

void
SvgAddQuadraticBezierSegment(svg_v2 StartP, svg_v2 EndP)
{
    svg_path_segement S;
    S.Type = SvgSegment_QuadraticBezier;
    // S.P1 = StartP;
    // S.P2 = EndP;
}

void
SvgAddEllipticSegment(svg_v2 StartP, svg_v2 EndP)
{
    svg_path_segement S;
    S.Type = SvgSegment_Elliptic;
    // S.P1 = StartP;
    // S.P2 = EndP;
}

void
SvgPathGetCommand(ls_string String)
{
    ls_parser P = String;

    svg_path_command_ CurrentCommand;
    svg_v2 CurrentP = {};

    while (P.RemainingBytes()) {
        if (ls_parser::Alpha(*P.At)) {
            CurrentCommand = CharCommandMap[*P.At];

            assert(CurrentCommand != SvgPathCommand_Null);

            if (CurrentCommand == SvgPathCommand_Move) {
                CurrentP = SvgParseV2(P);
                CurrentCommand = SvgPathCommand_LineTo;
            } else if (CurrentCommand == SvgPathCommand_MoveRel) {
                svg_v2 Pos = SvgParseV2(P);
                CurrentP.x += Pos.x;
                CurrentP.y += Pos.y;
                CurrentCommand = SvgPathCommand_LineToRel;
            } else if (CurrentCommand == SvgPathCommand_LineTo) {
                svg_v2 Pos = SvgParseV2(P);
                SvgAddLineSegment(CurrentP, Pos);
                CurrentP = Pos;
            } else if (CurrentCommand == SvgPathCommand_LineToRel) {
                svg_v2 Pos = SvgParseV2(P);
                Pos.x = CurrentP.x + Pos.x;
                Pos.y = CurrentP.y + Pos.y;

                SvgAddLineSegment(CurrentP, Pos);
                CurrentP = Pos;
            } else if (CurrentCommand == SvgPathCommand_HorizontalLine) {
                r32 X = SvgParseFloat(P);
                svg_v2 EndP = CurrentP;
                EndP.x = X;
                SvgAddLineSegment(CurrentP, EndP);
                CurrentP = EndP;
            } else if (CurrentCommand == SvgPathCommand_HorizontalLineRel) {
                r32 X = SvgParseFloat(P);
                svg_v2 EndP = CurrentP;
                EndP.x += X;
                SvgAddLineSegment(CurrentP, EndP);
                CurrentP = EndP;
            } else if (CurrentCommand == SvgPathCommand_VerticalLine) {
                r32 Y = SvgParseFloat(P);
                svg_v2 EndP = CurrentP;
                EndP.y = Y;
                SvgAddLineSegment(CurrentP, EndP);
                CurrentP = EndP;
            } else if (CurrentCommand == SvgPathCommand_VerticalLineRel) {
                r32 Y = SvgParseFloat(P);
                svg_v2 EndP = CurrentP;
                EndP.y += Y;
                SvgAddLineSegment(CurrentP, EndP);
                CurrentP = EndP;
            } else if (CurrentCommand == SvgPathCommand_CubicBezier) {
                svg_v2 Control1 = SvgParseV2(P);
                svg_v2 Control2 = SvgParseV2(P);
                svg_v2 EndP = SvgParseV2(P);
                SvgAddCubicBezierSegment(CurrentP, EndP, Control1, Control2);
                CurrentP = EndP;
            } else if (CurrentCommand == SvgPathCommand_CubicBezierRel) {
                svg_v2 Control1 = SvgParseV2(P);
                svg_v2 Control2 = SvgParseV2(P);
                svg_v2 EndP = SvgParseV2(P);

                Control1.x += CurrentP.x;
                Control1.y += CurrentP.y;

                Control2.x += CurrentP.x;
                Control2.y += CurrentP.y;

                EndP.x += CurrentP.x;
                EndP.y += CurrentP.y;

                SvgAddCubicBezierSegment(CurrentP, EndP, Control1, Control2);
                CurrentP = EndP;
            }
        }
    }
}

void
SvgParsePath(ls_string String)
{

}

void
SvgParseProperty(ls_string Tag, ls_string Prop, ls_string PropValue)
{
    if (Tag == "path") {
        if (PropValue == "d") {

        }
    }
}

svg
SvgParse(u8 *Data, u32 Size)
{
    CharCommandMap['M'] = SvgPathCommand_Move;
    CharCommandMap['L'] = SvgPathCommand_LineTo;
    CharCommandMap['H'] = SvgPathCommand_HorizontalLine;
    CharCommandMap['V'] = SvgPathCommand_VerticalLine;
    CharCommandMap['C'] = SvgPathCommand_CubicBezier;
    CharCommandMap['S'] = SvgPathCommand_SmoothCubicBezier;
    CharCommandMap['Q'] = SvgPathCommand_QuadraticBezier;
    CharCommandMap['T'] = SvgPathCommand_SmoothQuadraticBezier;
    CharCommandMap['A'] = SvgPathCommand_EllipticalArc;
    CharCommandMap['m'] = SvgPathCommand_MoveRel;
    CharCommandMap['l'] = SvgPathCommand_LineToRel;
    CharCommandMap['h'] = SvgPathCommand_HorizontalLineRel;
    CharCommandMap['v'] = SvgPathCommand_VerticalLineRel;
    CharCommandMap['c'] = SvgPathCommand_CubicBezierRel;
    CharCommandMap['s'] = SvgPathCommand_SmoothCubicBezierRel;
    CharCommandMap['q'] = SvgPathCommand_QuadraticBezierRel;
    CharCommandMap['t'] = SvgPathCommand_SmoothQuadraticBezierRel;
    CharCommandMap['a'] = SvgPathCommand_EllipticalArcRel;
    CharCommandMap['z'] = SvgPathCommand_ClosePath;
    CharCommandMap['Z'] = SvgPathCommand_ClosePath;

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

                ls_string Prop = Token.Text;

                Token = String.GetToken();
                assert(Token.Type == Token_String);

                SvgParseProperty(TagName, Prop, Token.Text);
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