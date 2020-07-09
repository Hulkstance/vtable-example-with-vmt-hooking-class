#include "misc.hpp"
#include "hooks.hpp"

class entity_t
{
public:
	virtual void print(std::string name) { std::cout << "Entity: " << name << std::endl; }
	virtual std::string get_name() { return "Unknown"; }
};

class player_t : public entity_t
{
public:
	void print(std::string name) override { std::cout << "Player: " << name << std::endl; }
	std::string get_name() override { return "Name"; }
};

void print_vtable(entity_t* entity)
{
	const auto vtable_pointer = *reinterpret_cast<uintptr_t**>(entity);

	std::cout << "vtable pointer - 0x" << std::uppercase << std::hex << vtable_pointer << std::endl;
	std::cout << "print function = 0x" << std::uppercase << std::hex << *vtable_pointer << std::endl;
	std::cout << "get_name function = 0x" << std::uppercase << std::hex << *(vtable_pointer + 1) << std::endl;
}

vmt_hook_t hooks;
int hooked_index = 0;

void __fastcall hk_print(entity_t* ecx, void* edx, std::string name)
{
	static auto fn = reinterpret_cast<void(__thiscall*)(entity_t*, std::string)>(hooks.get_method(hooked_index));
	fn(ecx, "Hooked");
}

int main()
{
	std::unique_ptr<entity_t> entity = std::make_unique<entity_t>();
	std::unique_ptr<entity_t> player = std::make_unique<player_t>();

	std::cout << "=====================" << std::endl;

	std::cout << "Printing vtable:" << std::endl << std::endl;
	print_vtable(entity.get());

	std::cout << "=====================" << std::endl;

	hooks.initialize(entity.get());
	hooks.replace_method(hooked_index, reinterpret_cast<uintptr_t>(&hk_print));
	hooks.hook();

	std::cout << "Unhooked vtable:" << std::endl << std::endl;
	entity->print("Player 1");
	player->print("Player 2");

	std::cout << "=====================" << std::endl;

	hooks.unhook();

	std::cout << "Unhooked vtable:" << std::endl << std::endl;
	entity->print("Player 1");
	player->print("Player 2");

	std::cout << "=====================" << std::endl;

	hooks.hook();

	std::cout << "Hooked vtable:" << std::endl << std::endl;
	entity->print("Player 1");
	player->print("Player 2");

	std::cout << "=====================" << std::endl;

	std::cin.get();
}