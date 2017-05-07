from django.shortcuts import render

def submit(request, submit_id):
	return render(request, "submit/submit.html")
