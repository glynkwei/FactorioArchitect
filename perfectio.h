#pragma once
#include "json.hpp"
#include <string>
#include <map>
#include <unordered_map>
#include <set>
#include <fstream>
#include <ostream>
struct boosts {
	double effectivity;
	double speed;
	boosts() : effectivity(0), speed(0) {

	}
	boosts(double eff, double spd) : effectivity(eff), speed(spd) {

	}
};
struct machine_config {
	double base_speed = 1;
    int type = 0;
	boosts intermediate_products_boost;
	boosts default_boosts;
	std::unordered_map<std::string, boosts> exception_table;
	machine_config(boosts int_bsts = boosts(), boosts def_bsts = boosts(), std::unordered_map<std::string, boosts> excpt_tble = std::unordered_map<std::string, boosts>()) :
		intermediate_products_boost(int_bsts), default_boosts(def_bsts), exception_table(excpt_tble) {

	}
};
enum solid_fuel_type {
	heavy, light, petroleum
};
enum refinery_type {
	basic, advanced, coal
};

struct production_config {
	machine_config assembling_config;
	machine_config chemical_config;
	solid_fuel_type fuel_type = petroleum;	//Convert solid fuel from heavy, light, or petroleum
	refinery_type ref_type = advanced;
	bool crack_when_possible = true;
	double oil_extraction_rate = 10;
	machine_config refinery_config;
	machine_config mining_config;
	machine_config pumpjack_config;
	machine_config smelting_config;
	machine_config rocket_config;
};
struct result {
	double count;		//how many machines
	double product;	//total products made
    std::string type;   //what machine is used
	result() : count(0), product(0) {

	}
};
struct order		{
	std::string item_name;
	double rate;
};

void to_json(nlohmann::json& j, const order& ord );
void from_json(const nlohmann::json &j, order& ord);
typedef std::map<std::string,result> name_result_table;
class factory {
	production_config config;
	nlohmann::json &data;
	std::set<std::string> items_to_share;
	std::vector<order> history;
	void h_plan(std::string item_name, name_result_table &root_planner,double rate = 1);
	
public:
    std::map<std::string, name_result_table> component_planners;
	name_result_table shared_planner;

	factory(std::string serialization, nlohmann::json &dat);
	factory(production_config pc, nlohmann::json &dat, std::set<std::string> planners_to_share = std::set<std::string>());
	void plan(order od);
	void plan(std::string item_name, double rate=1);
    std::string serialize();

};
std::ostream& operator<<(std::ostream& os, const factory& fy);
void to_json(nlohmann::json& j, const order& ord);
void from_json(const nlohmann::json& j, order& ord);
void to_json(nlohmann::json& j, const boosts& bsts);
void from_json(const nlohmann::json& j, boosts& bsts);
void to_json(nlohmann::json& j, const machine_config& config);
void from_json(const nlohmann::json& j, machine_config& config);
void to_json(nlohmann::json& j, const production_config& config);
void from_json(const nlohmann::json& j, production_config &config);


