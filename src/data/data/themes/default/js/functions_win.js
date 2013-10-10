/*
Desura is the leading indie game distribution platform
Copyright (C) 2011 Mark Chandler (Desura Net Pty Ltd)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>
*/

	function getBreadCrumbs() {
		return 'Settings|:|desura://settings';
	};
	
	curTheme = desura.settings.getValue('gc_theme');
	themes = [];
	allgames = [];
	games = [];
	gamesupdate = false;
	achtungShow = false;
	
	saveAll = function(){
		if($('#games .error').length > 0) {
			$.achtung({message: desura.utils.getLocalString("#SET_CIP_ERR_GAME"), className: 'achtungFail', timeout:5});
			$('#gamesettings ul.tabs a[href="#games"]').click();

			return false;
		} else {
			$.each($('input.setting_cvar:checkbox'), function(i, el){
				desura.settings.setValue($(el).attr('rel'), $(el).is(':checked')?'1':'0');
			});

			$.each($('input.setting_cvar:text'), function(i, el){
				desura.settings.setValue($(el).attr('rel'), $(el).val());
			});
			
			$('select.setting_cvar').each(function(i, el){		
				desura.settings.setValue($(this).attr('rel'), $(this).val());
			});

			desura.settings.setValue('gc_theme', curTheme);

			var out = [];
			
			$('input.txtpath').each(function() {
				out.push({id: $(this).attr('name'), path: $(this).val()});
			});
			
			desura.cip.saveList(out);

			if(gamesupdate) {
				$.achtung({message: desura.utils.getLocalString("#SET_CIP_SUCCESS_GAMES", "</a>", "<a href='#' onClick='desura.links.internalLink(0, desura.links.action.INSTALLEDW)'>", "<a href='#' onClick='desura.links.switchTab(desura.links.tab.ITEMS)'>"), className: 'achtungSuccess', timeout:0});
				gamesupdate = false;
			} else {
				$.achtung({message: desura.utils.getLocalString("#SET_CIP_SUCCESS"), className: 'achtungSuccess', timeout:5});
			}
		}

		return true;
	};

	onItemListUpdateCallBack = function() {
		if(achtungShow) {
			achtungShow.achtung("close");
		}

		allgames = desura.cip.getAllItems();
		allgames.sort(function(a, b) {
			var an = a.name.toLowerCase();
			var bn = b.name.toLowerCase();

			return (an < bn) ? -1 : (an > bn) ? 1 : 0;
		});

		for(x in allgames) {
			for(y in games) {
				if(games[y].id == allgames[x].id) {
					allgames.splice(x, 1);
				}
			}
		}

		$('#gameselect').html(parseTemplate($('#settings_games_select').html()));
	};

	onTabChange = function (tab) {
		$('#gamesettings > div').hide().filter(tab).show();

		$('#gamesettings ul.tabs a').removeClass('on');
		$('a[href="'+tab+'"]').addClass('on');
	};

	window.onhashchange = function () {
		if(window.location.hash) {
			onTabChange(window.location.hash);
		}
	};

	$(function () {
		games = desura.cip.getCurrentItems();
		themes = desura.settings.getThemes();
		performing = false;

		games.sort(function(a, b) {
			var an = a.name.toLowerCase();
			var bn = b.name.toLowerCase();

			return (an < bn) ? -1 : (an > bn) ? 1 : 0;
		});

		$('#content').html(parseTemplate($('#settings_tabs').html(), {}));

		onTabChange(window.location.hash ? window.location.hash : '#general');

		$('.setting_cvar').change(function() {
			$('input.btnsave').removeAttr('disabled');
		});

		$('input.btnsave').click(function() {
			if(saveAll()) {
				$('input.btnsave').attr('disabled', 'disabled');
			}
		});

		$('input.btnclose').click(function() {
			desura.links.switchTab(desura.links.tab.ITEMS);
		});

		$('input.btnbrowse').live('click', function() {
			path = desura.cip.browse($(this).parents('tr').find('label').text(), $(this).parents('td').find('.txtpath').val());

			$(this).parents('td').find('.txtpath').val(path).keyup().focus();
		});

		$('input.btndelete').live('click', function() {
			id = $(this).parents('td').attr('rel');

			$(this).parents('tr').remove();

			for(x in games) {
				if(games[x].id.toString() == id) {
					games.splice(x, 1);
					$('input.btnsave').removeAttr('disabled');
					gamesupdate = true;
					break;
				}
			}

			onItemListUpdateCallBack();
		});

		$('input.txtpath').live('keyup', function() {
			path = $(this).val();
			
			if(desura.cip.isValidPath(path) === true) {
				$(this).removeClass('error');

				$('input.btnsave').removeAttr('disabled');
				gamesupdate = true;
			} else {
				$(this).addClass('error');
			}
		});

		$('img.theme_subimage').click(function(){
			var index = $(this).attr('rel');
			curTheme = themes[index].name;

			$('img.theme_subimageon').removeClass('theme_subimageon');
			$('#theme_big_img').attr('src', themes[index].image);
			$('#theme_name').text(themes[index].displayName);
			$('#theme_author').text(themes[index].author);
			$('#theme_version').text(themes[index].version);
			$(this).addClass('theme_subimageon');

			$('input.btnsave').removeAttr('disabled');
		});

		$('#gameselectpath').keydown(function(){
			id = $('#gameselect').val();

			for(x in allgames) {
				if(allgames[x].id.toString() == id) {
					item = allgames[x];
					games.push(item);
					allgames.splice(x, 1);
					$('#gameselect :selected').remove();
					$('#gameselectadd').before(parseTemplate($('#settings_games_path').html()));
					$('#gamepath'+id).focus();	
					break;
				}
			}
		});

		$('#gameselectbrowse').click(function(){
			id = $('#gameselect').val();

			$('#gameselectpath').keydown();
			$('#gamepathrow'+id+' input.btnbrowse').click();
		});

		$('#gameselectrefresh').click(function(){
			achtungShow = $.achtung({message: desura.utils.getLocalString("#SET_CIP_UPDATE"), className: "achtungWait", icon: "wait-icon", timeout: 0});
			$(this).remove();
			desura.cip.updateList();
		});

		var checkLinkPath = function(){
			var name = $('#linkname').val();
			var exe = $('#linkexe').val();
			var isValidExe = desura.gamelink.isValidBinary(exe);
		
			if (isValidExe)
				$('#linkexe').removeClass('error');
			else
				$('#linkexe').addClass('error');
		
			if (exe == '' || name == '' || !isValidExe)
				$('#linkadd').attr('disabled', 'disabled');
			else
				$('#linkadd').attr('disabled', '');
		};
		
		$('#linkname').keyup(checkLinkPath);
		$('#linkexe').keyup(checkLinkPath);		
		
		$('#linkbrowse').click(function(){
			var name = $('#linkname').val();
			var exe = $('#linkexe').val();
			
			exe = desura.gamelink.browse(name, exe);
			$('#linkexe').val(exe);
			
			checkLinkPath();
		});	
		
		$('#linkadd').click(function(){
			var name = $('#linkname').val();
			var exe = $('#linkexe').val();
			
			desura.items.addLink(name, exe, "");
			
			$('#linkname').val('');
			$('#linkexe').val('');
			
			$('#linkadd').attr('disabled', 'disabled');
		});				
		
		$('#txtactivate').keyup(function(){
			if($(this).val() == '') {
				$('#btnactivate').attr('disabled', 'disabled');
			} else {
				$('#btnactivate').removeAttr('disabled');
			}
		});

		$('#activate').submit(function(){
			if($('#txtactivate').val() == '') {
				$.achtung({message: desura.utils.getLocalString("#SET_CIP_CDKEY"), className: 'achtungFail', timeout:5});
			} else {
				if(!performing) {
					performing = $.achtung({message: desura.utils.getLocalString("#SET_CIP_CDKEY_VALIDATING"), className: 'achtungWait', icon: 'wait-icon', timeout: 0});

					$.ajax({
						type: 'POST', url: $(this).attr('action'), timeout: 60000, dataType: 'json',
						data: {ajax: 't', h: $('#txtactivate').val(), b: 'x'},
						error: function(response, error, exception) {
							performing.achtung('close');
							performing = false;
							$.achtung({message: desura.utils.getLocalString("#SET_CIP_CDKEY_ERR"), className: 'achtungFail', timeout:5});
						},
						success: function(response) {
							performing.achtung('close');
							performing = false;

							if(response['success'] == true) {
								$.achtung({message: response['text'], className: 'achtungSuccess', timeout:typeof response['timeout']=='number'&&response['timeout']>0?response['timeout']:10});
								desura.utils.forceUpdatePoll();
							} else {
								$.achtung({message: response['text'], className: 'achtungFail', timeout:typeof response['timeout']=='number'&&response['timeout']>0?response['timeout']:10});
							}
							if(response['alert']) { $.achtung({message: response['alert']}); }
						}
					});
				}
			}

			return false;
		});

		desura.events.cip.onListUpdateEvent.register(onItemListUpdateCallBack, 'all');
		onItemListUpdateCallBack();

		if(allgames.length < 1) {
			desura.cip.updateList();
		}
		
		$('.setting_cvar, .show_tooltip').tipsy({gravity: 'w', fade: true, html: true});
	});
