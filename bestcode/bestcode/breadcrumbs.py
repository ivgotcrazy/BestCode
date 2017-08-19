
# [(name1, url1), (name2, url2), (name3, url3)]
def JumpTo(req, name, url):
	if 'breadcrumbs' not in req.session:
		req.session['breadcrumbs'] = []

	breadcrumbs = req.session['breadcrumbs']

	if len(breadcrumbs) == 0:
		breadcrumbs.append({'name':name, 'url':url})
	else:
		# jump to previos url, truncate
		found = False
		for index, item in enumerate(breadcrumbs):
			if item['url'] == url:
				found = True
				req.session['breadcrumbs'] = breadcrumbs[0:index+1]
				break

		# jump to new url, append
		if not found:
			breadcrumbs.append({'name':name, 'url':url})
		
def Clear(req):
	if 'breadcrumbs' in req.session:
		del req.session['breadcrumbs']	

def GetNavItems(req):
	if 'breadcrumbs' in req.session:
		return req.session['breadcrumbs']
	else:
		return []
