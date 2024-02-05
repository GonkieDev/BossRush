/* date = August 18th 2023 6:51 pm */

#ifndef UI_BUILDER_INCLUDE_H
#define UI_BUILDER_INCLUDE_H
//~

#define UI_Button(string) UI_Button_(S8Lit(string))
function UI_Widget *UI_ButtonMake(String8 string);
function UI_Comm UI_Button_(String8 string);

#define UI_Checkbox(pBool, string) UI_Checkbox_(pBool, S8Lit(string))
function UI_Comm UI_Checkbox_(b32 *pBool, String8 string);

#define UI_Text(string) UI_Text_(S8Lit(string))
function UI_Widget *UI_Text_(String8 text);
function UI_Widget *UI_TextF(char *fmt, ...);

function void UI_EvenSpacer(void);

#define UI_DropDownMenu(string, pShow) DeferLoop(UI_DropDownMenuStart(S8Lit(string), pShow), UI_DropDownMenuEnd())
#define UI_DropDownMenuMake(text, pShow) UI_DropDownMenuMake_(S8Lit(text), pShow)
function UI_Widget *UI_DropDownMenuMake_(String8 string, b32 *show);
function void UI_DropDownMenuStart(String8 string, b32 *show);
function void UI_DropDownMenuEnd(void);

function void UI_TextInput(String8 name, String8 *buf, u64 maxLen);

function void UI_ToolTipFromTarget(String8 text, UI_Widget *target);
function void UI_ToolTip(String8 text, HMM_Vec2 pos);

function void UI_Slider(String8 name, f32 *value, Rng1F32 limits);

//////////////////////////////////////////////
//~ End of file
#endif //UI_BUILDER_INCLUDE_H
