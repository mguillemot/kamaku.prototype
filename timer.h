#ifndef __TIMER
#define __TIMER

class Timer 
{ 
private: 
    //The clock time when the timer started 
    int startTicks; 
    
    //The ticks stored when the timer was paused 
    int pausedTicks; 
    
    //The timer status 
    bool paused; 
    bool started; 
    
public: 
    //Initializes variables 
    Timer(); 
    
    //The various clock actions 
    void start(); 
    void stop(); 
    void pause(); 
    void unpause();

	// Ticks number management
	void start_with_ticks(int tickNumber);
	void consume_ticks(int tickNumber);
    
    //Get the number of ticks since the timer started 
    //or gets the number of ticks when the timer was paused 
    int get_ticks(); 
    
    //Checks the status of the timer 
    bool is_started(); 
    bool is_paused(); 
};

#endif
