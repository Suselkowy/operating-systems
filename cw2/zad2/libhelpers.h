//closes provided file
void close_file(FILE* f);

//returns time in seconds from timespect
double get_time(struct timespec* timespec_s, struct timespec* timespec_e);

//saves time to pomiar_zad_2.txt file
void save_to_file(double time, char reverse_number);