namespace CSharpOnly
{
    class CSharpOnly
    {
        public static void Main(string[] args)
        {
            int val = Lib1.getResult();
            Lib2 l = new Lib2();
            val = val +  l.myVal + nested.Lib1.getResult();
            return;
        }
    }
}
