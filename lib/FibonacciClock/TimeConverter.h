class FibonacciValues {
  public:
    FibonacciValues() {

    }
    void setField(byte t, byte n, bool state) { // t == 0 == hour
      if( t <= 1 ) {
        this->fieldStates[n-1][t] = state;
      }
    }
    void setFieldHour(int n, bool state) {
      this->fieldStates[n-1][0] = state;
    }
    void setFieldMinute(int n, bool state) {
      this->fieldStates[n-1][1] = state;
    }

    bool getFieldHour(int n) {
      return this->fieldStates[n-1][0];
    }

    bool getFieldMinute(int n) {
      return this->fieldStates[n-1][1];
    }

    bool fieldStates[5][2] = {
      // H       M         H   M
        {false, false}, // 1   5
        {false, false}, // 1,  5
        {false, false}, // 2, 10
        {false, false}, // 3, 15
        {false, false}  // 5, 25
      };
};

class TimeConverter {
  public:
    TimeConverter(byte h, byte m) : hour(h), minute(m) {

    }
    FibonacciValues getFibonacciTime( void ) {
      this->calcFieldStated();
      return this->fs;
    }

  private:
    byte hour;
    byte minute;
    FibonacciValues fs;

    void calcFieldStated() {
      //hours:
        if( this->hour > 12 ) {
          this->hour -= 12; //normieren auf 0 bis 11 Stunden
        }

        if( this->hour - 5 >= 0 ) {
          fs.setFieldHour(5,true);
          this->hour -= 5;
        }

        if( this->hour - 3 >= 0 ) {
          fs.setFieldHour(4,true);
          this->hour -= 3;
        }

        if( this->hour - 2 >= 0 ) {
          fs.setFieldHour(3,true);
          this->hour -= 2;
        }

        if( this->hour - 1 >= 0 ) {
          fs.setFieldHour(2,true);
          this->hour -= 1;
        }

        if( this->hour - 1 == 0 ) {
          fs.setFieldHour(1,true);
          this->hour -= 1;
        }

        //minutes:
        if( this->minute - 25 >= 0 ) {
          fs.setFieldMinute(5,true);
          this->minute -= 25;
        }

        if( this->minute - 15 >= 0 ) {
          fs.setFieldMinute(4,true);
          this->minute -= 15;
        }

        if( this->minute - 10 >= 0 ) {
          fs.setFieldMinute(3,true);
          this->minute -= 10;
        }

        if( this->minute - 5 >= 0 ) {
          fs.setFieldMinute(2,true);
          this->minute -= 5;
        }

        if( this->minute - 5 == 0 ) {
          fs.setFieldMinute(1,true);
          this->minute -= 5;
        }

        //optimization
        //Split Box 2 Both to Box 1 and 2 for Minute
        if ((fs.getFieldMinute(2) && fs.getFieldHour(2)) && !fs.getFieldMinute(1))
        {
          fs.setFieldMinute(2,false);
          fs.setFieldMinute(1,true);
        }
        //Split Box 2 Both to Box 1 and 2 for Hour
        else if ((fs.getFieldMinute(2) && fs.getFieldHour(2)) && !fs.getFieldHour(1))
        {
          fs.setFieldHour(2,false);
          fs.setFieldHour(1,true);
        }

        //Split Box 3 Both to Box 1 and 2
        if ((fs.getFieldMinute(3) && fs.getFieldHour(3)) && (!fs.getFieldMinute(1) && !fs.getFieldMinute(2)))
        {
          fs.setFieldMinute(3,false);
          fs.setFieldMinute(2,true);
          fs.setFieldMinute(1,true);
        }
        else if ((fs.getFieldMinute(3) && fs.getFieldHour(3)) && (!fs.getFieldHour(1) && !fs.getFieldHour(2)))
        {
          fs.setFieldHour(3,false);
          fs.setFieldHour(2,true);
          fs.setFieldHour(1,true);
        }

        //Split Box 4 Both
        if ((fs.getFieldMinute(4) && fs.getFieldHour(4)) && (!fs.getFieldMinute(3) && (!fs.getFieldMinute(1) || !fs.getFieldMinute(2))))
        {
          fs.setFieldMinute(4,false);
          fs.setFieldMinute(3,true);

          if (!fs.getFieldMinute(2))
          {
            fs.setFieldMinute(2,true);
          }
          else
          {
            fs.setFieldMinute(1,true);
          }
        }
        else if ((fs.getFieldMinute(4) && fs.getFieldHour(4)) && (!fs.getFieldHour(3) && (!fs.getFieldHour(1) || !fs.getFieldHour(2))))
        {
          fs.setFieldHour(4,false);
          fs.setFieldHour(3,true);

          if (!fs.getFieldHour(2))
          {
            fs.setFieldHour(2,true);
          }
          else
          {
            fs.setFieldHour(1,true);
          }
        }

        //Split Box 5 Both
        if ((fs.getFieldMinute(5) && fs.getFieldHour(5)) &&
            (!fs.getFieldMinute(4) && (!fs.getFieldMinute(3) || (!fs.getFieldMinute(2) && !fs.getFieldMinute(1)))))
        {
          fs.setFieldMinute(5,false);
          fs.setFieldMinute(4,true);

          if (!fs.getFieldMinute(3))
          {
            fs.setFieldMinute(3,true);
          }
          else
          {
            fs.setFieldMinute(2,true);
            fs.setFieldMinute(1,true);
          }
        }
        else if ((fs.getFieldMinute(5) && fs.getFieldHour(5)) &&
            (!fs.getFieldHour(4) && (!fs.getFieldHour(3) || (!fs.getFieldHour(2) && !fs.getFieldHour(1)))))
        {
          fs.setFieldHour(5,false);
          fs.setFieldHour(4,true);

          if (!fs.getFieldHour(3))
          {
            fs.setFieldHour(3,true);
          }
          else
          {
            fs.setFieldHour(2,true);
            fs.setFieldHour(1,true);
          }
        }
    }
};
