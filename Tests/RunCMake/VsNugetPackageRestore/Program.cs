using System;
using CMake;

namespace Test
{
    public class Program
    {
        public static void Main(string[] args)
        {
            Console.WriteLine(NuGetTest.GetNumber());
            Console.ReadKey();
        }
    }
}
