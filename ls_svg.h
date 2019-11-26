#ifndef INCLUDE_GUARD_LS_SVG
#define INCLUDE_GUARD_LS_SVG

template <typename type>
struct svg_array {
    type *Data;
    u32 Count;
    u32 Cap;

    void FitN(u32 N) {
        if (!this->Data) {
            this->Cap = 10;
            this->Data = (type *)malloc(this->Cap * sizeof(type));
        }

        if (this->Count + N > this->Cap) {
            u32 NewCap = this->Cap * 2;
            this->Data = (type *)realloc(this->Data, NewCap);
            this->Cap = NewCap;
        }
    }

    type *AllocN(u32 N) {
        this->FitN(N);

        type *Result = this->Data + this->Count;
        *Result = {};
        this->Count += N;

        return Result;
    }

    void Push(type Data) {
        this->FitN(1);

        type *Result = this->Data + this->Count;
        *Result = Data;
        this->Count++;
    }
};

enum svg_element_ {
    SvgElement_Null,
    SvgElement_Rect,
    SvgElement_Circle,
    SvgElement_Ellipse,
    SvgElement_Line,
    SvgElement_Path,
    SvgElement_Count,
};

struct svg_v2 {
    r32 x,y;
};

enum svg_segment_ {
    SvgSegment_Line,
    SvgSegment_QuadraticBezier,
    SvgSegment_CubicBezier,
    SvgSegment_Elliptical,
};

struct svg_path_segement {
    svg_segment_ Type;

    union {
        struct {
            svg_v2 P1, P2, C1, C2;
        };
        struct {
            r32 Rx;
            r32 Ry;
            r32 Angle;
            char UseLargeArc;
            char Clockwise;
            svg_v2 EndP;
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
    svg_array<svg_path_segement> Segments;
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
    svg_array<svg_element> Elements;
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
SvgParseV2(ls_parser *P)
{
    token Token = P->GetToken();
    r32 x = Token.GetReal();

    Token = P->GetToken();
    r32 y = Token.GetReal();

    return {x,y};
}

r32
SvgParseFloat(ls_parser *P)
{
    token Token = P->GetToken();
    r32 Value = Token.GetReal();

    return Value;
}

int
SvgParseInt(ls_parser *P)
{
    token Token = P->GetToken();
    int Value = Token.GetInteger();
    return Value;
}

b32
SvgParseCommand(ls_parser *P, svg_path_command_ *Command)
{
    P->TrimLeft();

    if (ls_parser::Alpha(*P->At)) {
        *Command = CharCommandMap[*P->At];
        ++P->At;
        return true;
    }

    return false;
}

void
SvgAddLineSegment(svg_path *Path, svg_v2 StartP, svg_v2 EndP)
{
    svg_path_segement S;
    S.Type = SvgSegment_Line;
    S.P1 = StartP;
    S.P2 = EndP;

    Path->Segments.Push(S);
}

void
SvgAddCubicBezierSegment(svg_path *Path, svg_v2 StartP, svg_v2 EndP, svg_v2 Control1, svg_v2 Control2)
{
    svg_path_segement S;
    S.Type = SvgSegment_CubicBezier;
    S.P1 = StartP;
    S.P2 = Control1;
    S.C1 = EndP;
    S.C2 = Control2;

    Path->Segments.Push(S);
}

void
SvgAddQuadraticBezierSegment(svg_path *Path, svg_v2 StartP, svg_v2 EndP)
{
    svg_path_segement S;
    S.Type = SvgSegment_QuadraticBezier;

    Path->Segments.Push(S);
}

void
SvgAddEllipticalSegment(svg_path *Path, r32 Rx, r32 Ry, r32 Angle, b32 UseLargeArc, b32 Clockwise, svg_v2 Pos)
{
    svg_path_segement S;
    S.Type = SvgSegment_Elliptical;
    S.Rx = Rx;
    S.Ry = Ry;
    S.Angle = Angle;
    S.UseLargeArc = UseLargeArc;
    S.Clockwise = Clockwise;
    Path->Segments.Push(S);
}

void
SvgParsePath(svg *Svg, ls_string String)
{
    ls_parser P = String;

    svg_path_command_ CurrentCommand = SvgPathCommand_Null;
    svg_v2 CurrentP = {};
    svg_v2 PreviousControlP = {};

    auto *E = Svg->Elements.AllocN(1);
    E->Type = SvgElement_Path;

    printf("PATH:\n");

    while (P.RemainingBytes()) {
        SvgParseCommand(&P, &CurrentCommand);
        assert(CurrentCommand != SvgPathCommand_Null);

        switch (CurrentCommand) {
            case SvgPathCommand_Move: {
                printf("    Move ");
                CurrentP = SvgParseV2(&P);
                CurrentCommand = SvgPathCommand_LineTo;
                printf("%.2f %.2f\n", CurrentP.x, CurrentP.y);
            } break;
            case SvgPathCommand_MoveRel: {
                printf("    MoveRel ");
                svg_v2 Pos = SvgParseV2(&P);

                printf("%.2f %.2f\n", Pos.x, Pos.y);

                CurrentP.x += Pos.x;
                CurrentP.y += Pos.y;
                CurrentCommand = SvgPathCommand_LineToRel;
            } break;
            case SvgPathCommand_LineTo: {
                printf("    LineTo ");
                svg_v2 Pos = SvgParseV2(&P);
                printf("%.2f %.2f\n", Pos.x, Pos.y);
                SvgAddLineSegment(&E->Path, CurrentP, Pos);
                CurrentP = Pos;
            } break;
            case SvgPathCommand_LineToRel: {
                printf("    LineToRel ");
                svg_v2 Pos = SvgParseV2(&P);
                printf("%.2f %.2f\n", Pos.x, Pos.y);
                Pos.x = CurrentP.x + Pos.x;
                Pos.y = CurrentP.y + Pos.y;

                SvgAddLineSegment(&E->Path, CurrentP, Pos);
                CurrentP = Pos;
            } break;
            case SvgPathCommand_HorizontalLine: {
                printf("    HorizontalLine ");
                r32 X = SvgParseFloat(&P);
                svg_v2 EndP = CurrentP;
                EndP.x = X;
                SvgAddLineSegment(&E->Path, CurrentP, EndP);
                CurrentP = EndP;
            } break;
            case SvgPathCommand_HorizontalLineRel: {
                printf("    HorizontalLineRel ");
                r32 X = SvgParseFloat(&P);
                svg_v2 EndP = CurrentP;
                EndP.x += X;
                SvgAddLineSegment(&E->Path, CurrentP, EndP);
                CurrentP = EndP;
            } break;
            case SvgPathCommand_VerticalLine: {
                printf("    VerticalLine ");
                r32 Y = SvgParseFloat(&P);
                svg_v2 EndP = CurrentP;
                EndP.y = Y;
                SvgAddLineSegment(&E->Path, CurrentP, EndP);
                CurrentP = EndP;
            } break;
            case SvgPathCommand_VerticalLineRel: {
                printf("    VerticalLineRel ");
                r32 Y = SvgParseFloat(&P);
                svg_v2 EndP = CurrentP;
                EndP.y += Y;
                SvgAddLineSegment(&E->Path, CurrentP, EndP);
                CurrentP = EndP;
            } break;
            case SvgPathCommand_SmoothCubicBezier: {
                printf("    SmoofCubicBezier");
                svg_v2 Control2 = SvgParseV2(&P);
                svg_v2 EndP = SvgParseV2(&P);
                svg_v2 Control1 = CurrentP;

                svg_v2 C1Rel = PreviousControlP;
                C1Rel.x -= CurrentP.x;
                C1Rel.y -= CurrentP.y;
                C1Rel.x *= -1.0f;
                C1Rel.y *= -1.0f;

                Control1.x += C1Rel.x;
                Control1.y += C1Rel.y;

                PreviousControlP = Control2;

                printf("%.2f %.2f %.2f %.2f\n", Control2.x, Control2.y, EndP.x, EndP.y);
                SvgAddCubicBezierSegment(&E->Path, CurrentP, EndP, Control1, Control2);
                CurrentP = EndP;
            } break;
            case SvgPathCommand_SmoothCubicBezierRel: {
                printf("    SmoofCubicBezierRel");
                svg_v2 Control2 = SvgParseV2(&P);
                svg_v2 EndP = SvgParseV2(&P);
                svg_v2 Control1 = CurrentP;

                Control2.x += CurrentP.x;
                Control2.y += CurrentP.y;

                EndP.x += CurrentP.x;
                EndP.y += CurrentP.y;

                svg_v2 C1Rel = PreviousControlP;
                C1Rel.x -= CurrentP.x;
                C1Rel.y -= CurrentP.y;
                C1Rel.x *= -1.0f;
                C1Rel.y *= -1.0f;

                Control1.x += C1Rel.x;
                Control1.y += C1Rel.y;

                printf("%.2f %.2f %.2f %.2f\n", Control2.x, Control2.y, EndP.x, EndP.y);
                SvgAddCubicBezierSegment(&E->Path, CurrentP, EndP, Control1, Control2);
                CurrentP = EndP;
            } break;
            case SvgPathCommand_CubicBezier: {
                printf("    CubicBezier ");
                svg_v2 Control1 = SvgParseV2(&P);
                svg_v2 Control2 = SvgParseV2(&P);
                svg_v2 EndP = SvgParseV2(&P);

                PreviousControlP = Control2;

                printf("%.2f %.2f %.2f %.2f %.2f %.2f\n", Control1.x, Control1.y, Control2.x,Control2.y, EndP.x, EndP.y);
                SvgAddCubicBezierSegment(&E->Path, CurrentP, EndP, Control1, Control2);
                CurrentP = EndP;
            } break;
            case SvgPathCommand_CubicBezierRel: {
                printf("    CubicBezierRel ");
                svg_v2 Control1 = SvgParseV2(&P);
                svg_v2 Control2 = SvgParseV2(&P);
                svg_v2 EndP = SvgParseV2(&P);

                printf("%.2f %.2f %.2f %.2f %.2f %.2f\n", Control1.x, Control1.y, Control2.x,Control2.y, EndP.x, EndP.y);

                Control1.x += CurrentP.x;
                Control1.y += CurrentP.y;

                Control2.x += CurrentP.x;
                Control2.y += CurrentP.y;

                EndP.x += CurrentP.x;
                EndP.y += CurrentP.y;

                PreviousControlP = Control2;

                SvgAddCubicBezierSegment(&E->Path, CurrentP, EndP, Control1, Control2);
                CurrentP = EndP;
            } break;
            case SvgPathCommand_EllipticalArc: {
                printf("    EllipticalArc ");
                r32 Rx = SvgParseFloat(&P);
                r32 Ry = SvgParseFloat(&P);
                r32 Angle = SvgParseFloat(&P);
                int Arc = SvgParseInt(&P);
                int Sweep = SvgParseInt(&P);
                svg_v2 Pos = SvgParseV2(&P);

                printf("%.2f %.2f %.2f %d %d %.2f %.2f\n", Rx, Ry, Angle, Arc, Sweep, Pos.x, Pos.y);

                SvgAddEllipticalSegment(&E->Path, Rx, Ry, Angle, Arc, Sweep, Pos);
                CurrentP = Pos;
            } break;
            case SvgPathCommand_EllipticalArcRel: {
                printf("    EllipticalArcRel ");
                r32 Rx = SvgParseFloat(&P);
                r32 Ry = SvgParseFloat(&P);
                r32 Angle = SvgParseFloat(&P);
                int Arc = SvgParseInt(&P);
                int Sweep = SvgParseInt(&P);
                svg_v2 Pos = SvgParseV2(&P);

                printf("%.2f %.2f %.2f %d %d %.2f %.2f\n", Rx, Ry, Angle, Arc, Sweep, Pos.x, Pos.y);

                Pos.x += CurrentP.x;
                Pos.y += CurrentP.y;

                SvgAddEllipticalSegment(&E->Path, Rx, Ry, Angle, Arc, Sweep, Pos);
                CurrentP = Pos;
            } break;
            case SvgPathCommand_ClosePath: {
                printf("    ClosePath ");
                E->Path.Closed = true;

                E = Svg->Elements.AllocN(1);
                E->Type = SvgElement_Path;

                printf("PATH:\n");
            } break;

            default: {
                assert(!"AAAAAAAAAAAAAAAAA STAS VSE SLAMAL!!!!!");
            }
        }

        u32 a = 4;
    }
}

void
SvgParseProperty(svg *Svg, ls_string Tag, ls_string Prop, ls_string PropValue)
{
    if (Tag == "path") {
        if (Prop == "d") {
            SvgParsePath(Svg, PropValue);
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

                SvgParseProperty(&Svg, TagName, Prop, Token.Text);
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