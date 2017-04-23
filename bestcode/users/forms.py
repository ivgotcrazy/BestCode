from django import forms

class LoginForm(forms.Form):
    user_name = forms.CharField(label="用户名", max_length=64)
    user_pwd = forms.CharField(label="密码", max_length=64)

