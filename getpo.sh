#!/bin/sh

getnew(){
poname=po/epm.pot
if ( which xgettext 2>/dev/null ) ; then
  cat > "$poname" << EOF
msgid ""
msgstr ""
"Report-Msgid-Bugs-To: n.zdanova@drweb.com \n"
"POT-Creation-Date: 2010-02-04 13:47+0300\n"
"Last-Translator: Natalia Zdanova <n.zdanova@drweb.com>\n"
"MIME-Version: 1.0\n"
"Content-Type: text/plain; charset=UTF-8\n"
"Content-Transfer-Encoding: 8bit\n"

EOF

  files="setup.cxx setup2.cxx uninst.cxx uninst2.cxx uninst.sh.in"
  for f in $files ; do
     xgettext -o - $f | sed '1,18d' >> "$poname"
  done
msguniq "$poname" > "$poname".tmp
mv "$poname".tmp "$poname"
fi
}

merge(){
msgmerge po/ru.po po/epm.pot > po/ru_tmp.po
mv po/ru_tmp.po po/ru.po
}

gen(){
mkdir -p locale/ru/LC_MESSAGES
msgfmt -c --statistics -o locale/ru/LC_MESSAGES/epm.mo ru.po
}

getnew
#merge
#gen
