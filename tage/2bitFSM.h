#ifndef INC_2BITFSM_H
#define INC_2BITFSM_H

/**
* state = 0 Prediction Not Taken (strongly)
* state = 1 Prediction Taken (strongly)
* state = 2 Prediction Taken (weakly)
* state = 3 Prediction Not Taken (weakly)
*/
class FSM{

  char state = '2';
  public:

    void update_prediction(const bool outcome){
      if (outcome) {  // Branch was TAKEN
        if (state == '2' || state == '3') state = '1';
        if(state == '0') state = '3';
      } else {  // Branch was NOT TAKEN
        if (state == '2' || state == '3') state = '0';
        if(state == '1') state = '2';
      }

    }

    // true Taken, false Not Taken
  [[nodiscard]] bool get_prediction() const {
      if(state == '0'){
        return false;
      }
      else if (state == '1'){
        return true;
      }
      else if (state == '2'){
        return true;
      }
      else{
        return false;
      }

    }

  [[nodiscard]] char get_state() const {
    return state;
  }

  [[nodiscard]] bool is_weak() const {
      return state == '2' || state == '3';
    }

  [[nodiscard]] static int get_bit_size() {
      return 2;
    }

};

#endif //INC_2BITFSM_H
