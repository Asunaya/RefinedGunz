using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Xml;
using System.Xml.XmlConfiguration;
/*
 TODO:  randomize the zitem read, that way the same items arent written every time
 Also fix my write code, it's skipping elements
 Also, add multiple sets
*/
namespace GunGameBuilder
{
    public partial class Form1 : Form
    {
        public List<GunGame>[] GunGameItems = new List<GunGame>[]
        {
            new List<GunGame> { },
            new List<GunGame> { },
            new List<GunGame> { }
        };
        private static Random rng = new Random();

        public Form1()
        {
            InitializeComponent();
        }
        public void ReadZItem()
        {
            textBox2.AppendText("Reading ZItem");
            XmlReaderSettings settings = new XmlReaderSettings();
            settings.DtdProcessing = DtdProcessing.Parse;
            XmlReader reader = XmlReader.Create("zitem.xml", settings);
            if (reader == null)
                return;
            while(reader.Read())
            {
                //if (GunGameItems.Count > 300)
                //    break;
                GunGame ItemNode = new GunGame();
                reader.ReadToFollowing("ITEM");
                if (reader.GetAttribute("slot") == "melee")
                    ItemNode.Melee = Convert.ToInt32(reader.GetAttribute("id"));
                while (reader.ReadToNextSibling("ITEM"))
                {
                    if (reader.GetAttribute("slot") == "range")
                    {
                        ItemNode.Primary = Convert.ToInt32(reader.GetAttribute("id"));
                    }
                    break;
                }
                if (ItemNode.Primary != 0)
                {
                    while (reader.ReadToNextSibling("ITEM"))
                    {
                        if (reader.GetAttribute("slot") == "range")
                        {
                            ItemNode.Secondary = Convert.ToInt32(reader.GetAttribute("id")); break;
                        }
                    }
                }
                if (GunGameItems[0].Count < 100)
                    GunGameItems[0].Add(ItemNode);
                else
                {
                    if (GunGameItems[1].Count < 100)
                        GunGameItems[1].Add(ItemNode);
                    else
                    {
                        if (GunGameItems[2].Count < 100)
                            GunGameItems[2].Add(ItemNode);
                    }
                }
            }
            reader.Close();
        }
        public void WriteGunGame()
        {
            Shuffler.ListShuffler.Shuffle(GunGameItems[0], rng);
            Shuffler.ListShuffler.Shuffle(GunGameItems[1], rng);
            Shuffler.ListShuffler.Shuffle(GunGameItems[2], rng);
            textBox2.Clear();
            textBox2.AppendText("Writing GunGame");
            XmlWriterSettings writerSettings = new XmlWriterSettings();
            writerSettings.Indent = true;
            writerSettings.IndentChars = "\t";
            writerSettings.NewLineHandling = NewLineHandling.Entitize;
            writerSettings.NamespaceHandling = NamespaceHandling.Default;
            XmlWriter writer = XmlWriter.Create("gungame.xml", writerSettings);
            writer.WriteStartDocument();
            writer.WriteStartElement("XML");
            writer.WriteAttributeString("id", "gungame");
            writer.WriteStartElement("SET");
            writer.WriteAttributeString("id", Convert.ToString(1));

            foreach (var Item in GunGameItems)
            {
                for (int i = 0; i < 33; ++i)
                {
                    Random rand = new Random();
                    int Array = rand.Next(0, i);
                    writer.WriteStartElement("ITEMSET");
                    int melee = Item[i].Melee;
                    int primary = Item[i].Primary;
                    int secondary = Item[i].Secondary;
                    writer.WriteAttributeString("melee", melee.ToString());
                    writer.WriteAttributeString("primary", primary.ToString());
                    writer.WriteAttributeString("secondary", secondary.ToString());

                    writer.WriteEndElement();
                    string text = "ITEMSET: " + melee.ToString() + " " + primary.ToString() + " " +
                        secondary.ToString();
                    textBox1.AppendText(text);
                    textBox1.AppendText(Environment.NewLine);
                }
            }
        
            writer.WriteEndElement();
            writer.WriteStartElement("SET");
            writer.WriteAttributeString("id", Convert.ToString(2));
            foreach (var Item in GunGameItems)
            {
                for (int i = 0; i < 33; ++i)
                {
                    Random rand = new Random();
                    int Array = rand.Next(0, i);
                    writer.WriteStartElement("ITEMSET");
                    int melee = Item[Array].Melee;
                    int primary = Item[Array].Primary;
                    int secondary = Item[Array].Secondary;
                    writer.WriteAttributeString("melee", melee.ToString());
                    writer.WriteAttributeString("primary", primary.ToString());
                    writer.WriteAttributeString("secondary", secondary.ToString());

                    writer.WriteEndElement();
                    string text = "ITEMSET: " + melee.ToString() + " " + primary.ToString() + " " +
                        secondary.ToString();
                    textBox1.AppendText(text);
                    textBox1.AppendText(Environment.NewLine);
                }
            }
            writer.WriteEndElement();
            writer.WriteStartElement("SET");
            writer.WriteAttributeString("id", Convert.ToString(3));
            foreach (var Item in GunGameItems)
            {
                for (int i = 0; i < 33; ++i)
                {
                    Random rand = new Random();
                    int Array = rand.Next(0, i);
                    writer.WriteStartElement("ITEMSET");
                    int melee = Item[Array].Melee;
                    int primary = Item[Array].Primary;
                    int secondary = Item[Array].Secondary;
                    writer.WriteAttributeString("melee", melee.ToString());
                    writer.WriteAttributeString("primary", primary.ToString());
                    writer.WriteAttributeString("secondary", secondary.ToString());

                    writer.WriteEndElement();
                    string text = "ITEMSET: " + melee.ToString() + " " + primary.ToString() + " " +
                        secondary.ToString();
                    textBox1.AppendText(text);
                    textBox1.AppendText(Environment.NewLine);
                }
            }
            writer.WriteEndElement();
            writer.WriteEndElement();
            writer.WriteEndDocument();
            writer.Close();
            textBox2.Clear();
            textBox2.AppendText("Finished writing GunGame.xml");
            textBox1.AppendText("Thanks for using my gungame generator. Created by Jetman82");
        }

        private void writeGunGameToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ReadZItem();
            WriteGunGame();
        }

        private void aboutToolStripMenuItem_Click(object sender, EventArgs e)
        {
            AboutBox1 About = new GunGameBuilder.AboutBox1();

            About.Show();
        }
    }
}
