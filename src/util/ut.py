from lxml import etree
import sys
import os

if __name__ == "__main__":
    for f in sys.argv[1:]:
        fname, ext = os.path.splitext(f)
        print "Converting", f
        os.system("iconv -f ISO-8859-1 -t UTF-8 %s.xml > %s_utf.xml" % (fname, fname))
        print "Updating", f
        parser = etree.XMLParser(strip_cdata=False)
        root = etree.parse("%s_utf.xml" % fname, parser)

        for item in root.findall(".//TestCase"):
            for child in item.iterchildren():
                if child.tag not in ["Info", "FatalError", "Error", "TestingTime"]:
                    child.text = "[%s]\n[%s]" % (child.tag, child.text)
                    child.tag = "Info"

            if item.text:
                item.insert(0, etree.XML('<Info file="" line="1"><![CDATA[%s]]></Info>' % item.text, parser))
                item.text = ""
        
        open("%s_conv.xml" % fname, "w").write(etree.tostring(root, pretty_print=True))
