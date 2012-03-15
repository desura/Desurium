/*
 *  npw-config-template.h - nspluginwrapper configuration tool, template defs
 *
 *  nspluginwrapper (C) 2005-2009 Gwenole Beauchesne
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifdef ELF_CLASS
#ifndef ElfW
# if ELF_CLASS == ELFCLASS32
#  define ElfW(x)		Elf32_ ## x
#  define ELFW(x)		ELF32_ ## x
#  define FUNC(x)		x ## _32
#  define bswap_ad(x)	bswap_32(x)
# else
#  define ElfW(x)		Elf64_ ## x
#  define ELFW(x)		ELF64_ ## x
#  define FUNC(x)		x ## _64
#  define bswap_ad(x)	bswap_64(x)
# endif
#endif

static void FUNC(elf_swap_ehdr)(ElfW(Ehdr) *hdr)
{
  hdr->e_type			= bswap_16(hdr->e_type);
  hdr->e_machine		= bswap_16(hdr->e_machine);
  hdr->e_version		= bswap_32(hdr->e_version);
  hdr->e_entry			= bswap_ad(hdr->e_entry);
  hdr->e_phoff			= bswap_ad(hdr->e_phoff);
  hdr->e_shoff			= bswap_ad(hdr->e_shoff);
  hdr->e_flags			= bswap_32(hdr->e_flags);
  hdr->e_ehsize			= bswap_16(hdr->e_ehsize);
  hdr->e_phentsize		= bswap_16(hdr->e_phentsize);
  hdr->e_phnum			= bswap_16(hdr->e_phnum);
  hdr->e_shentsize		= bswap_16(hdr->e_shentsize);
  hdr->e_shnum			= bswap_16(hdr->e_shnum);
  hdr->e_shstrndx		= bswap_16(hdr->e_shstrndx);
}

static void FUNC(elf_swap_shdr)(ElfW(Shdr) *shdr)
{
  shdr->sh_name			= bswap_32(shdr->sh_name);
  shdr->sh_type			= bswap_32(shdr->sh_type);
  shdr->sh_flags		= bswap_ad(shdr->sh_flags);
  shdr->sh_addr			= bswap_ad(shdr->sh_addr);
  shdr->sh_offset		= bswap_ad(shdr->sh_offset);
  shdr->sh_size			= bswap_ad(shdr->sh_size);
  shdr->sh_link			= bswap_32(shdr->sh_link);
  shdr->sh_info			= bswap_32(shdr->sh_info);
  shdr->sh_addralign	= bswap_ad(shdr->sh_addralign);
  shdr->sh_entsize		= bswap_ad(shdr->sh_entsize);
}

static void FUNC(elf_swap_sym)(ElfW(Sym) *sym)
{
  sym->st_name			= bswap_32(sym->st_name);
  sym->st_value			= bswap_ad(sym->st_value);
  sym->st_size			= bswap_ad(sym->st_size);
  sym->st_shndx			= bswap_16(sym->st_shndx);
}

static bool FUNC(is_elf_plugin_fd)(int fd, NPW_PluginInfo *out_plugin_info)
{
  int i;
  bool ret = false;

  ElfW(Ehdr) ehdr;
  if (read(fd, &ehdr, sizeof(ehdr)) != sizeof(ehdr))
	return false;

  bool do_swap = (ehdr.e_ident[EI_DATA] == ELFDATA2LSB) && !is_little_endian();
  if (do_swap)
	FUNC(elf_swap_ehdr)(&ehdr);

  if (ehdr.e_ident[EI_CLASS] != ELF_CLASS)
	return false;
  if (ehdr.e_type != ET_DYN)
	return false;
  if (ehdr.e_version != EV_CURRENT)
	return false;

  ElfW(Shdr) *shdr = (ElfW(Shdr) *)load_data(fd, ehdr.e_shoff, ehdr.e_shnum * sizeof(*shdr));
  if (do_swap) {
	for (i = 0; i < ehdr.e_shnum; i++)
	  FUNC(elf_swap_shdr)(&shdr[i]);
  }

  char **sdata = (char **)calloc(ehdr.e_shnum, sizeof(*sdata));
  for (i = 0; i < ehdr.e_shnum; i++) {
	ElfW(Shdr) *sec = &shdr[i];
	if (sec->sh_type != SHT_NOBITS)
	  sdata[i] =  (char *)load_data(fd, sec->sh_offset, sec->sh_size);
  }

  ElfW(Shdr) *symtab_sec = NULL;
  for (i = 0; i < ehdr.e_shnum; i++) {
	ElfW(Shdr) *sec = &shdr[i];
	if (sec->sh_type == SHT_DYNSYM
		&& strcmp(sdata[ehdr.e_shstrndx] + sec->sh_name, ".dynsym") == 0) {
	  symtab_sec = sec;
	  break;
	}
  }
  if (symtab_sec == NULL)
	goto done;
  ElfW(Sym) *symtab = (ElfW(Sym) *)sdata[symtab_sec - shdr];
  char *strtab = sdata[symtab_sec->sh_link];

  int nb_syms = symtab_sec->sh_size / sizeof(*symtab);
  if (do_swap) {
	for (i = 0; i < nb_syms; i++)
	  FUNC(elf_swap_sym)(&symtab[i]);
  }

  int nb_np_syms;
  int is_wrapper_plugin = 0;
  for (i = 0, nb_np_syms = 0; i < nb_syms; i++) {
	ElfW(Sym) *sym = &symtab[i];
	const char *name = strtab + sym->st_name;
	if (ELFW(ST_BIND)(sym->st_info) != STB_GLOBAL)
	  continue;
	if (ELFW(ST_TYPE)(sym->st_info) == STT_OBJECT && strcmp(name, "NPW_Plugin") == 0)
	  is_wrapper_plugin = 1;
	if (ELFW(ST_TYPE)(sym->st_info) != STT_FUNC)
	  continue;
	if (!strcmp(name, "NP_GetMIMEDescription") ||
		!strcmp(name, "NP_Initialize") ||
		!strcmp(name, "NP_Shutdown"))
	  nb_np_syms++;
  }
  ret = (nb_np_syms == 3) && !is_wrapper_plugin;

 done:
  if (out_plugin_info) {
	const char *target_arch = NULL;
	switch (ehdr.e_machine) {
	case EM_386:			target_arch = "i386";	break;
	case EM_X86_64:			target_arch = "x86_64";	break;
	case EM_SPARC:			target_arch = "sparc";	break;
	case EM_PPC:			target_arch = "ppc";	break;
	case EM_PPC64:			target_arch = "ppc64";	break;
	}
	if (target_arch == NULL)
	  target_arch = "";
	strcpy(out_plugin_info->target_arch, target_arch);

	const char *target_os = NULL;
	switch (ehdr.e_ident[EI_OSABI]) {
	case ELFOSABI_LINUX:	target_os = "linux";	break;
	case ELFOSABI_SOLARIS:	target_os = "solaris";	break;
	case ELFOSABI_FREEBSD:	target_os = "freebsd";	break;
	}
	if (target_os == NULL) {
	  for (i = 0; i < ehdr.e_shnum; i++) {
	    ElfW(Shdr) *sec = &shdr[i];
	    if ((sec->sh_type == 0x6ffffffd /* SHT_SUNW_verdef */ || sec->sh_type == 0x6ffffffe /*SHT_SUNW_verneed*/)
			&& strcmp(sdata[ehdr.e_shstrndx] + sec->sh_name, ".SUNW_version") == 0) {
	      target_os = "solaris";
	      break;
	    }
	  }
	  if (target_os == NULL)
	    target_os = "";
	}
	strcpy(out_plugin_info->target_os, target_os);
  }

  for (i = 0; i < ehdr.e_shnum; i++)
	free(sdata[i]);
  free(sdata);
  free(shdr);
  return ret;
}

#undef bswap_ad
#undef ElfW
#undef ELFW
#undef FUNC
#undef ELF_CLASS
#endif
