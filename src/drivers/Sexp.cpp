/* -*- c++ -*-

   c2ffi
   Copyright (C) 2013  Ryan Pavlik

   This file is part of c2ffi.

   c2ffi is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 2 of the License, or
   (at your option) any later version.

   c2ffi is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with c2ffi.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "c2ffi.h"

using namespace c2ffi;

namespace c2ffi {
    class SexpOutputDriver : public OutputDriver {
        int _level;

        void endl() { if(_level <= 1) os() << std::endl; }

    public:
        SexpOutputDriver(std::ostream *os)
            : OutputDriver(os), _level(0) { }

        virtual void write_namespace(const std::string &ns) {
            os() << "(in-package :" << ns << ")" << std::endl;
        }

        virtual void write_comment(const char *str) {
            os() << ";; " << str << std::endl;
        }

        using OutputDriver::write;

        // Types -----------------------------------------------------------
        virtual void write(const SimpleType &t) {
            this->os() << t.name();
        }

        virtual void write(const BitfieldType &t) {
            os() << "(:bitfield " << t.width() << ")";
        }

        virtual void write(const PointerType& t) {
            _level++;
            this->os() << "(:pointer ";
            this->write(t.pointee());
            this->os() << ")";
            _level--;
        }

        virtual void write(const ArrayType &t) {
            _level++;
            os() << "(:array ";
            write(t.pointee());
            os() << " " << t.size() << ")";
            _level--;
        }

        virtual void write(const RecordType &t) {
            os() << "(";
            if(t.is_union())
                os() << ":union ";
            else
                os() << ":struct ";

            os() << t.name() << ")";
        }

        // Decls -----------------------------------------------------------
        virtual void write(const UnhandledDecl &d) {
            _level++;
            os() << ";; Unhandled: <" << d.kind() << "> " << d.name();
            endl();
            _level--;
        }

        virtual void write(const VarDecl &d) {
            _level++;
            if(d.is_extern())
                os() << "(extern ";
            else
                os() << "(const ";

            os() << d.name() << " ";
            write(d.type());

            if(d.value() != "")
                os() << " " << d.value();

            os() << ")";
            endl();
            _level--;
        }
        virtual void write(const FunctionDecl &d) {
            _level++;
            os() << "(function \"" << d.name() << "\" (";

            const NameTypeVector &params = d.fields();
            for(NameTypeVector::const_iterator i = params.begin();
                i != params.end(); i++) {
                if(i != params.begin())
                    os() << " ";

                os() << "(" << (*i).first;

                if((*i).first != "")
                    os() << " ";

                write(*(*i).second);
                os() << ")";
            }

            os() << ") ";
            write(d.return_type());
            os() << ")";
            endl();
            _level--;
        }

        virtual void write(const TypedefDecl &d) {
            _level++;
            os() << "(typedef " << d.name() << " ";
            write(d.type());
            os() << ")";
            endl();
            _level--;
        }

        virtual void write(const RecordDecl &d) {
            _level++;
            os() << "(";

            if(d.is_union())
                os() << "union ";
            else
                os() << "struct ";

            os() << d.name();

            const NameTypeVector &fields = d.fields();
            for(NameTypeVector::const_iterator i = fields.begin();
                i != fields.end(); i++) {
                os() << std::endl
                     << "    (" << i->first << " ";
                write(*(i->second));
                os() << ")";
            }

            os() << ")"; endl();
            _level--;
        }

        virtual void write(const EnumDecl &d) {
            _level++;
            os() << "(enum " << d.name();

            const NameNumVector &fields = d.fields();
            for(NameNumVector::const_iterator i = fields.begin();
                i != fields.end(); i++) {
                os() << std::endl
                     << "    (" << i->first << " " << i->second
                     << ")";
            }

            os() << ")"; endl();
            _level--;
        }
    };

    OutputDriver* MakeSexpOutputDriver(std::ostream *os) {
        return new SexpOutputDriver(os);
    }
}