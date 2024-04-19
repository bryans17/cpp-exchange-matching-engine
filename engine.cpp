#include <iostream>
#include <thread>
#include <cstring>

#include "io.hpp"
#include "engine.hpp"

void Engine::accept(ClientConnection connection)
{
	auto thread = std::thread(&Engine::connection_thread, this, std::move(connection));
	thread.detach();
}

void Engine::connection_thread(ClientConnection connection)
{
	while(true)
	{
		ClientCommand input {};
		switch(connection.readInput(input))
		{
			case ReadResult::Error: SyncCerr {} << "Error reading input" << std::endl;
			case ReadResult::EndOfFile: return;
			case ReadResult::Success: break;
		}

		if (input.type == input_cancel) {
			auto t = getTimestamp();
			if (!allOrders.count(input.order_id)) {
				Output::OrderDeleted(input.order_id, false, t);
        		return;
			}
			else std::strcpy(input.instrument, allOrders[input.order_id].instrument);
		} else allOrders[input.order_id] = input;

		// Functions for printing output actions in the prescribed format are
		// provided in the Output class:
		InstrBook& instrBook = instr_books[getInstrKey(input)];
		switch(input.type)
		{
			case input_buy: {
				instrBook.processBuy(input);
				break;
			}
			case input_sell: {
				instrBook.processSell(input);
				break;
			}
			case input_cancel: {
				instrBook.processCancel(input);
				break;
			}
			default: {
				SyncCerr {}
				    << "Got order: " << static_cast<char>(input.type) << " " << input.instrument << " x " << input.count << " @ "
				    << input.price << " ID: " << input.order_id << std::endl;

				// Remember to take timestamp at the appropriate time, or compute
				// an appropriate timestamp!
				auto output_time = getTimestamp();
				Output::OrderAdded(input.order_id, input.instrument, input.price, input.count, input.type == input_sell,
				    output_time);
				break;
			}
		}

		// Additionally:

		// Remember to take timestamp at the appropriate time, or compute
		// an appropriate timestamp!
		// intmax_t output_time = getCurrentTimestamp();

		// Check the parameter names in `io.hpp`.
		// Output::OrderExecuted(123, 124, 1, 2000, 10, output_time);
	}
}

Engine::instr_t Engine::getInstrKey(const ClientCommand& cmd)
{
	instr_t key = 0;
	for(int i=0; i<8; ++i) {
		if(cmd.instrument[i] == '\0') break;
		key = (key << 8) | cmd.instrument[i];
	}
	return key;
}
