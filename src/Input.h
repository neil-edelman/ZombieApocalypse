enum Toggle { t_off = 0, t_on };

enum Keys {
	k_unknown = 0,
	k_f1 = -127,
	k_f2,
	k_f3,
	k_f4,
	k_f5,
	k_f6,
	k_f7,
	k_f8,
	k_f9,
	k_f10,
	k_f11,
	k_f12,
	k_left,
	k_up,
	k_right,
	k_down,
	k_pgup,
	k_pgdn,
	k_home,
	k_end,
	k_ins
};

struct Input;
struct Widget;
struct Vector3f;

struct Input *Input(void);
void Input_(struct Input **inputptr);
void InputKey(struct Input *i, char k, enum Toggle doing);
void InputSetPosition(struct Input *i, const struct Vector3f *p);
struct Vector3f *InputGetPosition(struct Input *i);
void InputUpdate(struct Input *i, struct Widget *w);
