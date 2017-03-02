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
            while (reader.Read())
            {
                GunGame ItemNode = new GunGame();
                reader.ReadToFollowing("ITEM");
                if (reader.GetAttribute("slot") == "melee")
                {
                    ItemNode.melee = Convert.ToInt32(reader.GetAttribute("id"));
                    GunGameItems[0].Add(ItemNode);
                }
                if (reader.GetAttribute("slot") == "range")
                {
                    ItemNode.primary = Convert.ToInt32(reader.GetAttribute("id"));
                    GunGameItems[1].Add(ItemNode);
                }
                if (reader.GetAttribute("slot") == "range")
                {
                    ItemNode.secondary = Convert.ToInt32(reader.GetAttribute("id"));
                    GunGameItems[2].Add(ItemNode);
                }
            }
            reader.Close();
        }
        public void WriteGunGame()
        {
            Random rng = new Random();
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
            int setID = 1;
            for (int i = 0; i < GunGameItems.Length; ++i)
            {
                writer.WriteStartElement("SET");
                writer.WriteAttributeString("id", Convert.ToString(setID));
                for (int j = 0; j <= 100; ++j)
                {
                    writer.WriteStartElement("ITEMSET");
                    int melee = GunGameItems[0][j].melee;
                    int primary = GunGameItems[1][j].primary;
                    int secondary = GunGameItems[2][j].secondary;
                    writer.WriteAttributeString("melee", melee.ToString());
                    writer.WriteAttributeString("primary", primary.ToString());
                    writer.WriteAttributeString("secondary", secondary.ToString());

                    writer.WriteEndElement();
                    string text = "ITEMSET: " + melee.ToString() + " " + primary.ToString() + " " +
                        secondary.ToString();
                    textBox1.AppendText(text);
                    textBox1.AppendText(Environment.NewLine);
                }
                writer.WriteEndElement();
                ++setID;
            }

            writer.WriteEndElement();
            writer.WriteEndDocument();
            writer.Close();
            textBox2.Clear();
            textBox2.AppendText("Finished writing GunGame.xml");
            textBox1.AppendText("Thanks for using my gungame generator. Created by Jetman82");
            for (int i = 0; i < 3; ++i)
            {
                GunGameItems[i].Clear();
            }
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
