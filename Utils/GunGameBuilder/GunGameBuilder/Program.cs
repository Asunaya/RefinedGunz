using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace GunGameBuilder
{
    namespace Shuffler
    {
        public static class ListShuffler
        {
            public static void Shuffle<T>(this IList<T> list, Random rng)
            {
                int n = list.Count;
                while (n > 1)
                {
                    n--;
                    int k = rng.Next(n + 1);
                    T value = list[k];
                    list[k] = list[n];
                    list[n] = value;
                }
            }
        }
    }
    public class ZItem
    {
       public uint id;
       // public int 
        public string slot;
    }
   public class GunGame
    {
        int id;
        public int ID
        {
            get
            {
                return id;
            }
            set
            {
                id = value;
            }
        }
        int melee;
        public int Melee
        {
            get
            {
                return melee;
            }
            set
            {
                melee = value;
            }
        }
        int primary;
        public int Primary
        {
            get
            {
                return primary;
            }
            set
            {
                primary = value;
            }
        }
        int secondary;
        public int Secondary
        {
            get
            {
                return secondary;
            }
            set
            {
                secondary = value;
            }
        }
    }
    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new Form1());
        }
    }
}
