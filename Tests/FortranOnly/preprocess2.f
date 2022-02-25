#define int INTEGER
       ! This single unmatched quote ' should not cause an error during compilation
       PROGRAM PREPRO
       int f = 1
       PRINT*, f
       END
