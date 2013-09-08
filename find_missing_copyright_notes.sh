#!/bin/sh

repair_corrupted=""
excluded_commits=""
excluded_emails=""
name_substitutions="
	karolherbst:Karol Herbst
	Smilex:Ian T. Jacobsen
"
email_substitutions="
	lodle,mark@desura.com:Mark Chandler,mark@moddb.com
	mark,mark@ubuntu.(none):Mark Chandler,mark@moddb.com
	linux64,mark@ubuntu.(none):Mark Chandler,mark@moddb.com
"

license_header_begin="/\*
Desura is the leading indie game distribution platform"

license_header_end="This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
\(at your option\) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>
\*/"

# first we have to find every C style source file, we will do xml, html files later
find src/ build_out/ -type f -not -path "*/RES/*" -not -path "*src/branding_desur*" -not -path "*/third_party/*" -not -iname "*jquery*.js" -not -iname "*.crt" \( -iname "*.c*" -or -iname "*.h*" -or -iname "*.js*" -or -iname "*.css*" -or -iname "*.html*" \) | while read file; do

	printf "processing file: $file\n"

	# first we check if the file has a proper license header
	# the regex for the copyright must match something like: Copyright (C) 2011-2013,2015 Name Surname <email.adress@provider.com>\n
	# though we don't save the year information yet
	header=$(pcregrep -M "$license_header_begin[\n]*Copyright [a-zA-Z0-9<>@() :.,/\-\n]*$license_header_end" $file)
	# if we don't get a match, print the error out
	corrupted=""
	if [ -z "$header" ]; then
		printf "$file: license header missing or corrupted\n" 1>&2
		if [ -z "$repair_corrupted" ]; then
			continue
		fi
		corrupted="yes"
	fi

	# add the rest of the license text
	end_of_license_found=""
	cat $file | while IFS= read -r file_line || [ -n "$file_line" ]; do
		if [ -n "$end_of_license_found" ]; then
			printf "%s\n" "$file_line">> $file
			continue
		fi

		# if corrupted we just start at the beginngn and throw out an error
		if [ "$file_line" = "*/" ] || [ -n "$corrupted" ]; then
			if [ -n "$corrupted" ]; then
				printf "$file: was corrupted and yet repaired\n" 1>&2
			fi
			end_of_license_found="found"

			# first clear the file
			printf "" > "$file"

			if [ $(echo "$file" | awk -F . '{print $NF}') = html ]; then
				printf "<!--\n" >> "$file"
			fi

			# first write the license header into the file
			printf "$license_header_begin" | sed 's/\\//g' >> "$file"

			# now we just have to give us the formatted name <email> commit tag
			first=""
			commits=$(git log --no-merges --pretty=format:"%H:%an <%ae>" $file)
			for ec in $excluded_commits; do
				commits=$(printf "$commits" | sed "/$ec/d")
			done
			for email in $excluded_emails; do
				commits=$(printf "$commits" | sed "/$email/d")
			done

			# replace first commit
			commits=$(printf "$commits" | sed 's/6d1adee39a76a88e5bf2b877f61a83a5b0eced51:Mark Chandler <mark@moddb.com>/6d1adee39a76a88e5bf2b877f61a83a5b0eced51:Desura Ltd. <support@desura.com>/g')

			# substitutes
			commits=$(
				printf "$name_substitutions" | {
					while read -r subst; do
						if [ -z "$subst" ]; then
							continue
						fi
						old=$(printf "$subst" | cut -d: -f1)
						new=$(printf "$subst" | cut -d: -f2)
						commits=$(printf "$commits" | sed "s/$old /$new /g")
					done
					printf "$commits"
				}
			)
			commits=$(
				printf "$email_substitutions" | {
					while read subst; do
						if [ -z "$subst" ]; then
							continue
						fi
						old=$(printf "$subst" | cut -d: -f1 )
						oldname=$(printf "$old" | cut -d, -f1 )
						oldemail=$(printf "$old" | cut -d, -f2 )
						new=$(printf "$subst" | cut -d: -f2 )
						newname=$(printf "$new" | cut -d, -f1 )
						newemail=$(printf "$new" | cut -d, -f2 )

						commits=$(echo "$commits" | sed "s/$oldname <$oldemail>/$newname <$newemail>/g")
					done
					printf "$commits"
				}
			)

			printf "$commits" | sort -u --key=1.41 | while read -r author; do
				# and write our copyright string
				if [ -z "$first" ]; then
					printf "\nCopyright " >> "$file"
					first="true"
				else
					printf "\n          " >> "$file"
				fi

				author=$(printf "$author" | cut -d: -f2)
				printf "(C) $author" >> "$file"
			done
			printf "\n\n$license_header_end\n" | sed 's/\\//g' >> "$file"
		fi
	done

done

