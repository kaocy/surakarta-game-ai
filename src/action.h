#pragma once
#include <algorithm>
#include <unordered_map>
#include <string>
#include "board.h"

class Action {
public:
	Action(unsigned code = -1u) : code(code) {}
	Action(const Action& a) : code(a.code) {}
	virtual ~Action() {}

	class Eat; // create a eating action
	class Move; // create a moving action

public:
	virtual int apply(Board& b) const {
		auto proto = entries().find(type());
		if (proto != entries().end()) return proto->second->reinterpret(this).apply(b);
		return -1;
	}
	virtual std::ostream& operator >>(std::ostream& out) const {
        auto proto = entries().find(type());
        if (proto != entries().end()) return proto->second->reinterpret(this) >> out;
        return out << "??";
    }
    virtual std::istream& operator <<(std::istream& in) {
        auto state = in.rdstate();
        for (auto proto = entries().begin(); proto != entries().end(); proto++) {
            if (proto->second->reinterpret(this) << in) return in;
            in.clear(state);
        }
        return in.ignore(2);
    }

public:
	operator unsigned() const { return code; }
	unsigned type() const { return code & type_flag(-1u); }
	unsigned origin() const { return code & 0b111111; }
	unsigned destination() const { return (code >> 6) & 0b111111; }

protected:
	static constexpr unsigned type_flag(unsigned v) { return v << 24; }

	typedef std::unordered_map<unsigned, Action*> prototype;
	static prototype& entries() { static prototype m; return m; }
	virtual Action& reinterpret(const Action* a) const { return *new (const_cast<Action*>(a)) Action(*a); }

	unsigned code;
};

class Action::Eat : public Action {
public:
	static constexpr unsigned type = type_flag('e');
	Eat(unsigned ori, unsigned dest) : Action(Eat::type | (ori & 0b111111) | ((dest & 0b111111) << 6)) {}
	Eat(unsigned code) : Action(Eat::type | code) {}
	Eat(const Action& a = {}) : Action(a) {}

public:
	int apply(Board& b) const {
		return b.eat(origin(), destination());
	}

protected:
	Action& reinterpret(const Action* a) const { return *new (const_cast<Action*>(a)) Eat(*a); }
	static __attribute__((constructor)) void init() { entries()[type_flag('e')] = new Eat; }
};

class Action::Move : public Action {
public:
	static constexpr unsigned type = type_flag('m');
	Move(unsigned ori, unsigned dest) : Action(Move::type | (ori & 0b111111) | ((dest & 0b111111) << 6)) {}
	Move(unsigned code) : Action(Move::type | code) {}
	Move(const Action& a = {}) : Action(a) {}

public:
	int apply(Board& b) const {
		return b.move(origin(), destination());
	}

protected:
	Action& reinterpret(const Action* a) const { return *new (const_cast<Action*>(a)) Move(*a); }
	static __attribute__((constructor)) void init() { entries()[type_flag('m')] = new Move; }
};
