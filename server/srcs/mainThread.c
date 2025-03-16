#include "../incs/main.h"
#if 0
void mainThread(int *state, int *wheelchair, int *people)
{
    while (1)
    {
        switch (*state)
        {
            case (ARRIVED_DOWNSTAIRS):
            {
                sleep(5);
                *state = GET_RFID;
            }

            case (GET_RFID):
            {
                while (1)
                {
                    if (*state == BUTTON_PRESSED)
                    {
                        if (checkInAndOut(wheelchair) == 0)
                        {
                            *state = SPEAKER_YIELD;
                            break;
                        }
                        else
                        {
                            *state = ELEVATOR_START;
                            break;
                        }
                    }
                }
            }
        }
    }
}

bool checkInAndOut(int *wheelChair)
{
    if (*wheelChair == 1)
        return (false);
    else
        return (true);
}
#endif