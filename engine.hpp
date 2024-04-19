// This file contains declarations for the main Engine class. You will
// need to add declarations to this file as you develop your Engine.

#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <atomic>
#include <chrono>

#include "io.hpp"
#include "lockedmap.hpp"
#include "instrbook.hpp"

struct Engine
{
	using instr_t = long long;
public:
	void accept(ClientConnection conn);

private:
	void connection_thread(ClientConnection conn);
	instr_t getInstrKey(const ClientCommand& cmd);
	LockedMap<instr_t, InstrBook> instr_books;
	LockedMap<uint32_t, ClientCommand> allOrders;
};

#endif
