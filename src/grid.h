#ifndef GRID_H
#define GRID_H
/**
 * Difference grid types require different handling
 * for boundary conditions
 */
enum Grid_Type
{
    Density,
    X_Velocity,
    Y_Velocity, 
    None
}; 

template <typename T>
struct Fluid_Grid {
    T* array_;
    Grid_Type type_;
    int N_;

    Fluid_Grid(int N, Grid_Type type = None) : N_(N), type_(type) {
        array_ = new T[(N+2)*(N+2)];
        
        // Zero out array 
        set_all(0);
    } 

    /** Zero out the internal array */
    void reset() {
        // Zero out array 
        set_all(0);
    }

    /** Resize the internal array, then zero out new array */
    void resize(int N) {
        N_ = N;
        delete[] array_;
        array_ = new T[(N+2)*(N+2)];
        reset();
    }

    /** Set all values of the array to some value, v */
    void set_all(T v) {
		std::fill(array_, array_ + (N_+2)*(N_+2), v);
    }

    Fluid_Grid(const Fluid_Grid&) = delete;
    Fluid_Grid& operator = (const Fluid_Grid&) = delete;

    ~Fluid_Grid() {
        delete[] array_;
    }

    /** 2D access operator */
    T& operator () (int i, int j) { 
        return array_[i + (N_+2) * j]; 
    }    

    void debug_print () {
        printf("----- DEBUG PRINT -----\n");
        for (int i = 1; i <= N_; ++i) {
            for (int j = 1; j <= N_; ++j) {
                printf("%.3f ", (*this)(i,j));
            }
            printf("\n");
        }
        printf("\n");
    }
};

#endif // GRID_H
