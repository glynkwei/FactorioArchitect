#include "json.hpp"
#include "perfectio.h"
#include <iostream>
#include <iomanip>

using json = nlohmann::json;
using namespace std;

int result_amount(json &item, json &data) {
	int result_count = 0;
	if (item.find("result_count") != item.end()) {
		result_count = item["result_count"];
	}
	else if (item.find("results") != item.end()) {
		//iterate through results to find name
		for (auto res_it = item["results"].begin(); res_it != item["results"].end(); ++res_it) {
			json res = *res_it;
			//check for weird types as well
			if (res["name"] == item["name"] ||
				(res["name"] == "solid-fuel" && item["name"] == "solid-fuel-from-petroleum-gas") ||
				(res["name"] == "solid-fuel" && item["name"] == "solid-fuel-from-light-oil") ||
				(res["name"] == "solid-fuel" && item["name"] == "solid-fuel-from-heavy-oil"))	{
				result_count += res["amount"];
			}
		}
		
	}
	else {
		result_count = 1;
	}
	return result_count;
}
json recipe(string item_name, json &data) {
	if (data["recipe"][item_name].is_null()) {
		//is resource type
		return nullptr;
	}
	if (data["recipe"][item_name]["normal"].is_null()) {
		return data["recipe"][item_name];
	}
	else {
		return data["recipe"][item_name]["normal"];
	}
}
void to_json(json& j, const order& ord) {
	j = json{{ "name",ord.item_name }, { "rate", ord.rate }};
}

void from_json(const json& j, order& ord) {
	ord.item_name = j.at("name").get<string>();
	ord.rate = j.at("rate").get<double>();
}

void to_json(json& j, const boosts& bsts)   {
    j = json{bsts.effectivity,bsts.speed};
}
void from_json(const json& j, boosts& bsts)  {
    bsts.effectivity= j.at(0).get<double>();
    bsts.speed = j.at(1).get<double>();
}

void to_json(json& j, const machine_config& config)     {

    j = json{{"baseSpd" , config.base_speed},
            {"type", config.type},
            {"intBoosts" , json(config.intermediate_products_boost)},
            {"defBoosts", json(config.default_boosts)}};
}
void from_json(const json& j, machine_config& config) {
    config.base_speed=j.at("baseSpd").get<double>();
    config.type = j.at("type").get<int>();
    config.intermediate_products_boost = j.at("intBoosts").get<boosts>();
    config.default_boosts = j.at("defBoosts").get<boosts>();
}

void to_json(json& j, const production_config& config)  {
j = json{{"mining", config.mining_config},
            {"smelting",config.smelting_config},
            {"pumpjack",config.pumpjack_config},
            {"refinery",config.refinery_config},
            {"chemistry",config.chemical_config},
            {"assembling",config.assembling_config},
            {"rocketry",config.rocket_config},
            {"crackWhenPossible",config.crack_when_possible},
            {"refiningType",static_cast<int>(config.ref_type)}};
}
void from_json(const json& j, production_config &config) {
    config.mining_config = j.at("mining").get<machine_config>();
    config.smelting_config = j.at("smelting").get<machine_config>();
    config.pumpjack_config = j.at("pumpjack").get<machine_config>();
    config.refinery_config = j.at("refinery").get<machine_config>();
    config.chemical_config = j.at("chemistry").get<machine_config>();
    config.assembling_config = j.at("assembling").get<machine_config>();
    config.rocket_config = j.at("rocketry").get<machine_config>();
    config.crack_when_possible = j.at("crackWhenPossible").get<bool>();
    config.ref_type = static_cast<refinery_type>(j.at("refiningType").get<int>());
}


factory::factory(string serialization, json &dat) : data(dat) {
	json j = json::parse(serialization);
	if (!j.is_array()) {
		throw 0;
		//Serialization not valid
	}
	else {
		for (auto p : j) {
			order ord = p;
			plan(ord);
		}
	}
}
factory::factory(production_config pc,  json &dat, set<string> share) : config(pc), items_to_share(share),data(dat)	{
}
void factory::plan(order od) {
	plan(od.item_name, od.rate);
}
void factory::plan(std::string item_name, double rate) {
	history.push_back({ item_name,rate });
	name_result_table &root_planner = items_to_share.find(item_name) == items_to_share.end() ? 
										component_planners[item_name] :
										shared_planner;
	h_plan(item_name, root_planner, rate);
}
string factory::serialize()	{
    json save;
    save["sharedItem"] = json(items_to_share);
    save["config"] = json(config);
    save["orders"] = json(history);
    stringstream stream;
    stream << save;
    return stream.str();
}

void factory::h_plan(std::string item_name, name_result_table &root_planner, double rate)	{
	string effective_name = item_name;
	if (item_name == "solid-fuel") {
		if (config.fuel_type == petroleum) {
			effective_name = "solid-fuel-from-petroleum-gas";
		}
		else if (config.fuel_type == light) {
			effective_name = "solid-fuel-from-light-oil";
		}
		else {
			effective_name = "solid-fuel-from-heavy-oil";
		}
	}
	else if (item_name == "light-oil" || item_name == "heavy-oil") {
        //only include rate, set -1 to count
        name_result_table &planner = items_to_share.find(item_name) == items_to_share.end() ?
            root_planner : shared_planner;
        planner[item_name].product = rate;
        planner[item_name].count = -1;
		return;
    }
	else if (item_name == "petroleum-gas") {

		double s = 1 + config.chemical_config.intermediate_products_boost.effectivity / 100;
		double c = config.chemical_config.base_speed;
		double t = c*(1 + config.chemical_config.intermediate_products_boost.speed / 100);

		double r = 1 + config.refinery_config.intermediate_products_boost.effectivity / 100;
		double v = config.refinery_config.base_speed;
		double u = v*(1 + config.refinery_config.intermediate_products_boost.speed / 100);
		if (config.ref_type == advanced) {
			if (config.crack_when_possible) {
				

				double k = (6 * s + s*s + 11)*r;
				double refineries = rate / (u*k);
				double heavy_to_light = rate *0.15 / (t*k);
				double light_oil_cracked = heavy_to_light * 10 * t  * s;
				double light_to_petrol = rate *(0.9 + 0.15*s) / (t * k);
				double petroleum_cracked = light_to_petrol * 20.0 / 3.0 * t * s;
				double total_water = 10 * (refineries*u + heavy_to_light*t + light_to_petrol*t);
				double total_oil = refineries * 20 * u;
				double petroleum_processed = refineries * 11 * r*u;
				h_plan("water", root_planner, total_water);
				h_plan("crude-oil", root_planner, total_oil);
				/////////////////////////////
				{
					string process = "heavy-oil-cracking-to-light-oil";
					name_result_table &planner = items_to_share.find(process) == items_to_share.end() ?
						root_planner : shared_planner;
					planner[process].count += heavy_to_light;
					planner[process].product += light_oil_cracked;
                    planner[process].type = "chemistry";
				}
				////////////////////////////
				{
					string process = "light-oil-cracking-to-petroleum-gas";
					name_result_table &planner = items_to_share.find(process) == items_to_share.end() ?
						root_planner : shared_planner;
					planner[process].count += light_to_petrol;
					planner[process].product += petroleum_cracked;
                    planner[process].type = "chemistry";
				}
				///////////////////////////
				{
					string process = "advanced-oil-processing";
					name_result_table &planner = items_to_share.find(process) == items_to_share.end() ?
						root_planner : shared_planner;
					planner[process].count += refineries;
					planner[process].product += petroleum_processed;
                    planner[process].type = "refinery";
				}
			}
			else {
				//no cracking, note there will be excessive light oil/heavy oil!
				string process = "advanced-oil-processing";
				name_result_table &planner = items_to_share.find(process) == items_to_share.end() ?
					root_planner : shared_planner;
				double count = rate / (11 * r*u);
				double water_rate = rate * 10 / (11 * r);
                h_plan("water", planner, water_rate);
				planner[process].count += count;
				planner[process].product += rate;
                planner[process].type = "refinery";
			}
		}
		else if (config.ref_type == basic) {
			if (config.crack_when_possible) {
				double k = (4 * s + 3*s*s + 8)*r;
				double refineries = rate / (u*k);
				double heavy_to_light = rate *0.45 / (t*k);
				double light_oil_cracked = heavy_to_light * 10 * t  * s;
				double light_to_petrol = rate *(0.6 + 0.45*s) / (t * k);
				double petroleum_cracked = light_to_petrol * 20.0 / 3.0 * t * s;
				double total_water = 10 * (heavy_to_light*t + light_to_petrol*t);
				double total_oil = refineries * 20 * u;
				double petroleum_processed = refineries * 8 * r*u;
				h_plan("water", root_planner, total_water);
				h_plan("crude-oil", root_planner, total_oil);
				{
					string process = "heavy-oil-cracking-to-light-oil";
					name_result_table &planner = items_to_share.find(process) == items_to_share.end() ?
						root_planner : shared_planner;
					planner[process].count += heavy_to_light;
					planner[process].product += light_oil_cracked;
                    planner[process].type = "chemistry";
				}
				////////////////////////////
				{
					string process = "light-oil-cracking-to-petroleum-gas";
					name_result_table &planner = items_to_share.find(process) == items_to_share.end() ?
						root_planner : shared_planner;
					planner[process].count += light_to_petrol;
					planner[process].product += petroleum_cracked;
                    planner[process].type = "chemistry";
				}
				///////////////////////////
				{
					string process = "basic-oil-processing";
					name_result_table &planner = items_to_share.find(process) == items_to_share.end() ?
						root_planner : shared_planner;
					planner[process].count += refineries;
					planner[process].product += petroleum_processed;
                    planner[process].type = "refinery";
				}
			}
			else {
				//no cracking, note there will be excessive light oil/heavy oil!
				string process = "basic-oil-processing";
				name_result_table &planner = items_to_share.find(process) == items_to_share.end() ?
					root_planner : shared_planner;
				double count = rate / (8 * r*u);
				planner[process].count += count;
				planner[process].product += rate;
                planner[process].type = "refinery";
			}
		}
		else if (config.ref_type == coal) {
			if (config.crack_when_possible) {
				double k = (2 * s + s*s + 4)*r;
				double refineries = rate / (u*k);
				double heavy_to_light = rate *0.15 / (t*k);
				double light_oil_cracked = heavy_to_light * 10 * t  * s;
				double light_to_petrol = rate *(0.3 + 0.15*s) / (t * k);
				double petroleum_cracked = light_to_petrol * 20.0 / 3.0 * t * s;
				double total_water = 10 * (heavy_to_light*t + light_to_petrol*t);
				double total_steam = refineries * 10 * u;
				double total_coal = refineries * 2 * u;
				double petroleum_processed = refineries * 4 * r*u;
				h_plan("water", root_planner, total_water);
				h_plan("steam", root_planner, total_steam);
                h_plan("coal", root_planner, total_coal);
				{
					string process = "heavy-oil-cracking-to-light-oil";
					name_result_table &planner = items_to_share.find(process) == items_to_share.end() ?
						root_planner : shared_planner;
					planner[process].count += heavy_to_light;
					planner[process].product += light_oil_cracked;
                    planner[process].type = "chemistry";
				}
				////////////////////////////
				{
					string process = "light-oil-cracking-to-petroleum-gas";
					name_result_table &planner = items_to_share.find(process) == items_to_share.end() ?
						root_planner : shared_planner;
					planner[process].count += light_to_petrol;
					planner[process].product += petroleum_cracked;
                    planner[process].type = "chemistry";
				}
				///////////////////////////
				{
					string process = "coal-liquefaction";
					name_result_table &planner = items_to_share.find(process) == items_to_share.end() ?
						root_planner : shared_planner;
					planner[process].count += refineries;
					planner[process].product += petroleum_processed;
                    planner[process].type = "refinery";
				}
			}
			else {
				//no cracking, note there will be excessive light oil/heavy oil!
				string process = "coal-liquefaction";
				name_result_table &planner = items_to_share.find(process) == items_to_share.end() ?
					root_planner : shared_planner;
				double count = rate / (4 * r*u);
				double steam_rate = rate * 2.5 / r;
				plan("steam", steam_rate);
				planner[process].count += count;
				planner[process].product += rate;
                planner[process].type = "refinery";
			}
		}
		else {
			throw 0;
			//Type not supported yet.
		}
		return;
	}
	else if (item_name == "crude-oil") {
		auto boost = config.pumpjack_config.intermediate_products_boost;
		double count = rate / (config.pumpjack_config.base_speed*config.oil_extraction_rate*(1 + boost.effectivity / 100)*(1 + boost.speed / 100));
		name_result_table &planner = items_to_share.find(item_name) == items_to_share.end() ?
			root_planner : shared_planner;
		planner[item_name].count += count;
		planner[item_name].product += rate;
        planner[item_name].type = "pumpjack";
		return;
	}
	else if (item_name == "water") {
		double count = rate / 1200;
		name_result_table &planner = items_to_share.find(item_name) == items_to_share.end() ?
			root_planner : shared_planner;
		planner[item_name].count += count;
		planner[item_name].product += rate;
         planner[item_name].type = "offshore-pump";
		return;
	}
	else if (item_name == "steam") {
		double count = rate / 60;
		name_result_table &planner = items_to_share.find(item_name) == items_to_share.end() ?
			root_planner : shared_planner;
		planner[item_name].count += count;
		planner[item_name].product += rate;
         planner[item_name].type = "boiler";
		h_plan("water", root_planner, rate);
        return;
	}
	else  if (item_name == "iron-ore" || item_name == "copper-ore" || item_name == "coal" || item_name == "stone") {
		auto boost = config.mining_config.intermediate_products_boost;
		double true_speed;
		if (item_name != "stone") {
			if (config.mining_config.base_speed == 0) {
				true_speed = .28;
			}
			else if (config.mining_config.base_speed == 1) {
				true_speed = .525;
			}
			else {
				//Type not supported yet.
				throw 0;
			}

		}
		else {
			if (config.mining_config.base_speed == 0) {
				true_speed = .3675;
			}
			else if (config.mining_config.base_speed == 1) {
				true_speed = .65;
			}
			else {
				//Type not supported yet.
				throw 0;
			}
		}
		double count = rate / (true_speed*(1 + boost.effectivity / 100)*(1 + boost.speed / 100));
		name_result_table &planner = items_to_share.find(item_name) == items_to_share.end() ?
			root_planner : shared_planner;
		planner[item_name].count += count;
		planner[item_name].product += rate;
         planner[item_name].type = "mining";
		return;
	}
	json item = recipe(effective_name, data);
	if (item.is_null()) {
		//is resource or something weird

	}
	int result_count = result_amount(item, data);
	double energy_required = item.find("energy_required") == item.end() ? 0.5 : item["energy_required"];
	boosts item_boost;
    double base;
	json limitations = data["module"]["productivity-module"]["limitation"];
	bool use_productivity = false;

	for (auto &limit : limitations) {
		if (limit == effective_name) {
			use_productivity = true;
		}
	}
    string type;

    auto _item = data.at("recipe").at(effective_name);
    if (_item.find("category") != _item.end()) {
        if (_item["category"] == "chemistry") {
            //chemistry
            type = "chemistry";
            if (use_productivity) {
                item_boost = config.chemical_config.intermediate_products_boost;
            }
            else {
                item_boost = config.assembling_config.default_boosts;
            }
            base = config.assembling_config.base_speed;
        }
        else if (_item["category"] == "smelting") {
            //smelting
            type = "smelting";
            if (use_productivity) {
                item_boost = config.smelting_config.intermediate_products_boost;
            }
            else {
                item_boost = config.smelting_config.default_boosts;
            }
            base = config.smelting_config.base_speed;
        }
        else if (_item["category"] == "rocket-building") {
            //rocket-building
            type = "rocketry";
            if (use_productivity) {
                item_boost = config.rocket_config.intermediate_products_boost;
            }
            else {
                item_boost = config.rocket_config.default_boosts;
            }
            base = config.rocket_config.base_speed;
        }
        else {
            //assembling
            type = "assembling";
            if (use_productivity) {
                item_boost = config.assembling_config.intermediate_products_boost;
            }
            else {
                item_boost = config.assembling_config.default_boosts;
            }
            base = config.assembling_config.base_speed;
        }
    }
    else if (item.find("category") != item.end()) {
		if (item["category"] == "chemistry") {
			//chemistry
            type = "chemistry";
			if (use_productivity) {
				item_boost = config.chemical_config.intermediate_products_boost;
			}
			else {
				item_boost = config.assembling_config.default_boosts;
			}
			base = config.assembling_config.base_speed;
		}
		else if (item["category"] == "smelting") {
			//smelting
            type = "smelting";
			if (use_productivity) {
				item_boost = config.smelting_config.intermediate_products_boost;
			}
			else {
				item_boost = config.smelting_config.default_boosts;
			}
			base = config.smelting_config.base_speed;
		}
		else if (item["category"] == "rocket-building") {
			//rocket-building
            type = "rocketry";
			if (use_productivity) {
				item_boost = config.rocket_config.intermediate_products_boost;
			}
			else {
				item_boost = config.rocket_config.default_boosts;
			}
			base = config.rocket_config.base_speed;
		}
		else {
			//assembling
            type = "assembling";
			if (use_productivity) {
				item_boost = config.assembling_config.intermediate_products_boost;
			}
			else {
				item_boost = config.assembling_config.default_boosts;
			}
			base = config.assembling_config.base_speed;
		}
	}
	else {
		//assembling
        type = "assembling";
		if (use_productivity) {
			item_boost = config.assembling_config.intermediate_products_boost;
		}
		else {
			item_boost = config.assembling_config.default_boosts;
		}
		base = config.assembling_config.base_speed;
	}
	double effective_rate = rate / ((1 + item_boost.effectivity / 100)*result_count);
	double count = effective_rate*energy_required / (base*(1 + item_boost.speed / 100));
	name_result_table &planner = items_to_share.find(item_name) == items_to_share.end() ?
		root_planner : shared_planner;
	planner[item_name].count += count;
	planner[item_name].product += rate;
    planner[item_name].type = type;
	//count ingredients
	json ingredients = item["ingredients"];
	for (auto it = ingredients.begin(); it != ingredients.end(); ++it) {
		json ingredient = *it;
		string ingredient_name;
		int ingredient_count;
		if (ingredient.is_array()) {
			string name = ingredient[0];
			ingredient_name = name;
			ingredient_count = ingredient[1];
		}
		else if (ingredient.is_object()) {
			string name = ingredient["name"];
			ingredient_name = name;
			ingredient_count = ingredient["amount"];
		}
		else {
			throw 0;
		}
		double ingredient_weight = effective_rate * ingredient_count;
		h_plan(ingredient_name, root_planner, ingredient_weight);
	} 
}

std::ostream& operator<<(std::ostream& os, const factory& fy)
{
	//components first
	for (auto component : fy.component_planners) {
		os.width(40);
		os << component.first << endl;
		os.width(40);
		os << "Name" << "\t:\t";
		os.width(20);
		os << "Rate" << "\t";
		os.width(20);
		os << "Total machines" << endl;
		for (auto entry : component.second) {
			os.width(40);
			os << fixed;
			os.precision(3);
			os << entry.first << "\t:\t";
			os.width(20);
			os << entry.second.product << "\t";
			os.width(20);
			os << entry.second.count << endl;
		}
		os << endl;
	}
	//Shared
	os.width(40);
	os << "Name" << "\t:\t";
	os.width(20);
	os << "Rate" << "\t";
	os.width(20);
	os << "Total machines" << endl;
	for (auto entry : fy.shared_planner) {
		os.width(40);
		os << fixed;
		os.precision(3);
		os << entry.first << "\t:\t";
		os.width(20);
		os << entry.second.product << "\t";
		os.width(20);
		os << entry.second.count << endl;
	}
	os << endl;
	return os;
}


