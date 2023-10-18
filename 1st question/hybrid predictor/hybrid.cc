#include <map>
#include "msl/fwcounter.h"
#include "ooo_cpu.h"
#include <cmath>
#include "tage.h"
#include <iostream>
bool tp = false, pp = false;
int per_out = 0;

namespace
{
#perceptron pred
constexpr std::size_t PERCEPTRON_HISTORY = 143; // history length for the global history shift register
constexpr std::size_t PERCEPTRON_BITS = 8;     // number of bits per weight
constexpr std::size_t NUM_PERCEPTRONS = 100;
constexpr std::size_t NUM_UPDATE_ENTRIES = 100;

#meta predictor
constexpr std::size_t META_PRED_ENTRIES= 4099;
#constexpr std::size_t BIMODAL_PRIME = 4099;
constexpr std::size_t COUNTER_BITS = 2;

std::map<O3_CPU*, std::array<champsim::msl::fwcounter<COUNTER_BITS>, BIMODAL_TABLE_SIZE>> bimodal_table;
} // namespace

namespace
{
template <std::size_t HISTLEN, std::size_t BITS>
class perceptron
{
  champsim::msl::sfwcounter<BITS> bias{0};
  std::array<champsim::msl::sfwcounter<BITS>, HISTLEN> weights = {};

public:
  
  void update(bool result, std::bitset<HISTLEN> history)
  {
    // if the branch was taken, increment the bias weight, else decrement it, with saturating arithmetic
    bias += result ? 1 : -1;

    // for each weight and corresponding bit in the history register...
    auto upd_mask = result ? history : ~history; // if the i'th bit in the history positively
                                                 // correlates with this branch outcome,
    for (std::size_t i = 0; i < std::size(upd_mask); i++) {
      // increment the corresponding weight, else decrement it, with saturating arithmetic
      weights[i] += upd_mask[i] ? 1 : -1;
    }
  }
};
 // size of buffer for keeping 'perceptron_state' for update

/* 'perceptron_state' - stores the branch prediction and keeps information
 * such as output and history needed for updating the perceptron predictor
 */
struct perceptron_state {
  uint64_t ip = 0;
  bool prediction = false;                     // prediction: 1 for taken, 0 for not taken
  long long int output = 0;                    // perceptron output
  std::bitset<PERCEPTRON_HISTORY> history = 0; // value of the history register yielding this prediction
};

std::map<O3_CPU*, std::array<perceptron<PERCEPTRON_HISTORY, PERCEPTRON_BITS>,
                             NUM_PERCEPTRONS>> perceptrons;             // table of perceptrons
std::map<O3_CPU*, std::deque<perceptron_state>> perceptron_state_buf;   // state for updating perceptron predictor
std::map<O3_CPU*, std::bitset<PERCEPTRON_HISTORY>> spec_global_history; // speculative global history - updated by predictor
std::map<O3_CPU*, std::bitset<PERCEPTRON_HISTORY>> global_history;      // real global history - updated when the predictor is
                                                                        // updated
} // namespace

//
// part of perceptron predictor ends here
auto predict(std::bitset<HISTLEN> history)
  {
    auto output = bias.value();

    // find the (rest of the) dot product of the history register and the perceptron weights.
    for (std::size_t i = 0; i < std::size(history); i++) {
      if (history[i])
        output += weights[i].value();
      else
        output -= weights[i].value();
    }

    return output;
  }



void O3_CPU::initialize_branch_predictor() {
  std::cout << "\nUsing hybrid (tage and perceptron) branch predictor\n";  
  tage_init();
   
}

// take perceptron if 1x, tage if 0x 
uint8_t O3_CPU::predict_branch(uint64_t ip)
{
  auto hash = ip % ::BIMODAL_PRIME;
  auto value = ::bimodal_table[this][hash];

  tp = tage_predict(ip);

  // predict using perceptron
  auto index = ip % ::NUM_PERCEPTRONS;
  per_out = ::perceptrons[this][index].predict(::spec_global_history[this]);

  bool prediction = (per_out >= 0);

  // record the various values needed to update the predictor

  pp = prediction;

  if (value >= 2) return pp;
  return tp;
}

void O3_CPU::last_branch_result(uint64_t ip, uint64_t branch_target, uint8_t taken, uint8_t branch_type)
{

  auto hash = ip % ::BIMODAL_PRIME;

  // ::bimodal_table[this][hash] += taken ? 1 : -1;


// sklearn
// xgboost -- xgbR
// gradientboosting
// svr
// k means regressor

  // taken -- 
  if (::bimodal_table[this][hash] >= 2) { // we selected perceptron
    if ((taken and pp) or (!taken and !pp)) { // correctly predicted
      ::bimodal_table[this][hash] += 1;
    } else { // wrongly predicted
      ::bimodal_table[this][hash] -= 1; 
    }
  } else { // we selected tage
    if ((taken and tp) or (!taken and !tp)) { // correctly predicted
      ::bimodal_table[this][hash] -= 1;
    } else { // wrongly predicted
      ::bimodal_table[this][hash] += 1; 
    }
  }

  



  // first update tage, because perceptron may return early
  // 
  tage_train(ip,taken);


  // start of update of perceptron

  ::perceptron_state_buf[this].push_back({ip, pp, per_out, ::spec_global_history[this]});
  if (std::size(::perceptron_state_buf[this]) > ::NUM_UPDATE_ENTRIES)
  ::perceptron_state_buf[this].pop_front();

  // update the speculative global history register
  ::spec_global_history[this] <<= 1;
  ::spec_global_history[this].set(0, pp);


  auto state = std::find_if(std::begin(::perceptron_state_buf[this]), std::end(::perceptron_state_buf[this]), [ip](auto x) { return x.ip == ip; });
  if (state == std::end(::perceptron_state_buf[this]))
    return; // Skip update because state was lost

  auto [_ip, prediction, output, history] = *state;
  ::perceptron_state_buf[this].erase(state);

  auto index = ip % ::NUM_PERCEPTRONS;

  // update the real global history shift register
  ::global_history[this] <<= 1;
  ::global_history[this].set(0, taken);

  // if this branch was mispredicted, restore the speculative history to the
  // last known real history
  if (prediction != taken)
    ::spec_global_history[this] = ::global_history[this];

  // if the output of the perceptron predictor is outside of the range
  // [-THETA,THETA] *and* the prediction was correct, then we don't need to
  // adjust the weights
  const int THETA = std::floor(1.93 * PERCEPTRON_HISTORY + 14); // threshold for training
  if ((output <= THETA && output >= -THETA) || (prediction != taken))
    ::perceptrons[this][index].update(taken, history);

  // end of update of perceptron

}

